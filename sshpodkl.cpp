#include "sshpodkl.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QInputDialog>
#include <QDebug>

namespace {
// опции, отключающие интерактивные вопросы SSH при первом коннекте
const QStringList kSshOpts{
    "-o", "StrictHostKeyChecking=no",
    "-o", "UserKnownHostsFile=/dev/null",
    "-o", "BatchMode=yes" // не запрашивать пароль интерактивно
};

QStringList addSshOpts(const QStringList &base)
{
    QStringList out = kSshOpts;
    out << base;
    return out;
}
} // anonymous namespace

/************************  вспомогательная работа с JSON  ************************/
static QJsonDocument loadJsonFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(f.readAll());
}
static bool saveJsonFile(const QString &path, const QJsonDocument &doc)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    return f.write(doc.toJson(QJsonDocument::Indented)) > 0;
}

/************************  конструктор  ************************/
SSHPodkl::SSHPodkl(QObject *parent) : QObject(parent)
{
    QDir().mkpath(PASSHIVE_ROOT);
    loadRegistry();
}

/************************  публичные методы  ************************/
bool SSHPodkl::ensureConnection(const QString &login, char dbType, const QString &accountPassword)
{
    m_key = accountPassword;

    if (!m_registry.contains(login))
        if (!bootstrapNewUser(login, dbType)) return false;

    QJsonObject obj = m_registry.value(login).toObject();
    m_conn.ip   = obj["ip"].toString();
    m_conn.user = obj["user"].toString();
    QString dt  = obj["dbType"].toString();
    m_conn.dbType = dt.isEmpty() ? dbType : dt.at(0).toLatin1();

    QByteArray cipher = QByteArray::fromBase64(obj["pass"].toString().toLatin1());
    m_conn.plainPass  = decrypt(cipher);

    return initDatabase();
}

QString SSHPodkl::ssh(const QString &cmd) const
{
    QString host = QString("%1@%2").arg(m_conn.user, m_conn.ip);

    // если пароль известен — используем sshpass, иначе рассчитываем на ключ
    if (!m_conn.plainPass.isEmpty()) {
        QStringList args = {"-p", m_conn.plainPass, "ssh"};
        args += addSshOpts({host, cmd});
        return runCmd("sshpass", args, 30000);
    }
    return runCmd("ssh", addSshOpts({host, cmd}), 30000);
}

bool SSHPodkl::sftpUpload(const QString &localPath, const QString &remotePath) const
{
    QString dst = QString("%1@%2:%3").arg(m_conn.user, m_conn.ip, remotePath);
    if (!m_conn.plainPass.isEmpty()) {
        QStringList args = {"-p", m_conn.plainPass, "scp"};
        args += addSshOpts({localPath, dst});
        return !runCmd("sshpass", args, 30000).isNull();
    }
    return !runCmd("scp", addSshOpts({localPath, dst}), 30000).isNull();
}

/************************  CRUD  ************************/
// (код CRUD операций без изменений)

/************************  registry  ************************/
bool SSHPodkl::loadRegistry()
{
    QJsonDocument doc = loadJsonFile(PASSHIVE_ROOT + "/users.json");
    m_registry = doc.isObject() ? doc.object() : QJsonObject{};
    return true;
}

bool SSHPodkl::saveRegistry()
{
    return saveJsonFile(PASSHIVE_ROOT + "/users.json", QJsonDocument(m_registry));
}

/************************  bootstrap  ************************/
bool SSHPodkl::bootstrapNewUser(const QString &login, char dbType)
{
    QString ip   = QInputDialog::getText(nullptr, "IP", "IP сервера:");
    QString user = QInputDialog::getText(nullptr, "SSH user", "Логин:");
    QString pass = QInputDialog::getText(nullptr, "SSH pass", "Пароль:", QLineEdit::Password);
    if (ip.isEmpty() || user.isEmpty() || pass.isEmpty()) return false;

    // первый пробный вызов ssh через sshpass с отключённой проверкой ключа
    QString host = QString("%1@%2").arg(user, ip);
    QStringList testArgs = {"-p", pass, "ssh"};
    testArgs += addSshOpts({host, "echo ok"});
    if (!runCmd("sshpass", testArgs, 15000).contains("ok")) return false;

    QString os; if (!detectOS(os)) return false;
    installPackagesForOS(os);
    createSystemUser(login);

    QByteArray cipher = encrypt(pass.toUtf8());
    QJsonObject e; e["ip"] = ip; e["user"] = user; e["pass"] = QString(cipher.toBase64()); e["dbType"] = QString(dbType);
    m_registry[login] = e; saveRegistry();
    return true;
}

/************************  утилиты  ************************/
bool SSHPodkl::detectOS(QString &outOs)
{
    QString r = runCmd("cat", {"/etc/os-release"});
    outOs = r.contains("Ubuntu") ? "ubuntu" : (r.contains("Debian") ? "debian" : "linux");
    return true;
}

bool SSHPodkl::installPackagesForOS(const QString &os)
{
    QString cmd = (os == "ubuntu" || os == "debian") ? "sudo apt-get -y install postgresql sqlite3" : "";
    return cmd.isEmpty() || !ssh(cmd).isNull();
}

bool SSHPodkl::createSystemUser(const QString &login)
{
    return !ssh(QString("sudo useradd -m -s /usr/sbin/nologin passhive_%1 || true").arg(login)).isNull();
}

bool SSHPodkl::initDatabase()
{
    if (m_conn.dbType == 'p') {
        ssh("psql -lqt | cut -d '|' -f 1 | grep -qw PassHiveDB || createdb PassHiveDB");
        ssh("psql -d PassHiveDB -c \"CREATE TABLE IF NOT EXISTS PassHiveDB (id SERIAL PRIMARY KEY, \"user\" TEXT, name BYTEA, pass BYTEA, notice BYTEA);\"");
        return true;
    }
    QString dbPath = QString("%1/pass_hive_%2.db").arg(PASSHIVE_ROOT, m_conn.user);
    ssh(QString("test -f %1 || sqlite3 %1 \"CREATE TABLE PassHive (id INTEGER PRIMARY KEY AUTOINCREMENT, name BLOB, pass BLOB, notice BLOB);\"").arg(dbPath));
    return true;
}

/************************  шифрование  ************************/
QByteArray SSHPodkl::encrypt(const QByteArray &plain)
{
    return m_pm.zakod(QString::fromUtf8(plain), m_key.toUtf8()).toUtf8();
}

QByteArray SSHPodkl::decrypt(const QByteArray &cipher)
{
    return m_pm.raskod(QString::fromUtf8(cipher), m_key.toUtf8()).toUtf8();
}

/************************  оболочка над QProcess  ************************/
QString SSHPodkl::runCmd(const QString &program, const QStringList &args, int timeoutMs) const
{
    QProcess p;
    p.setProgram(program);
    p.setArguments(args);
    p.setProcessChannelMode(QProcess::MergedChannels); // забираем stdout+stderr
    p.start();
    if (!p.waitForFinished(timeoutMs <= 0 ? 30000 : timeoutMs)) { // 30 секунд по умолчанию
        p.kill();
        return {};
    }
    return QString::fromUtf8(p.readAllStandardOutput());
}
