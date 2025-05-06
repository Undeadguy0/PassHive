
#include "sshpodkl.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QInputDialog>
#include <QDebug>


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


SSHPodkl::SSHPodkl(QObject *parent) : QObject(parent)
{
    QDir().mkpath(PASSHIVE_ROOT);
    loadRegistry();
}


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
    return runCmd("ssh", {host, cmd});
}

bool SSHPodkl::sftpUpload(const QString &localPath, const QString &remotePath) const
{
    QString dst = QString("%1@%2:%3").arg(m_conn.user, m_conn.ip, remotePath);
    return !runCmd("scp", {localPath, dst}).isNull();
}


bool SSHPodkl::insertPassword(const QString &forUser,
                              const QString &name,
                              const QString &pass,
                              const QString &notice)
{
    QByteArray encName   = encrypt(name.toUtf8()).toBase64();
    QByteArray encPass   = encrypt(pass.toUtf8()).toBase64();
    QByteArray encNotice = encrypt(notice.toUtf8()).toBase64();

    if (m_conn.dbType == 'p') {
        QString sql = QString("psql -d PassHiveDB -c \"INSERT INTO PassHiveDB (\"user\", name, pass, notice) VALUES ('%1', decode('%2','base64'), decode('%3','base64'), decode('%4','base64'));\"")
        .arg(forUser, QString(encName), QString(encPass), QString(encNotice));
        return !ssh(sql).isNull();
    }
    QString dbPath = QString("%1/pass_hive_%2.db").arg(PASSHIVE_ROOT, m_conn.user);
    QString sql = QString("sqlite3 %1 \"INSERT INTO PassHive (name, pass, notice) VALUES (x'%2', x'%3', x'%4');\"")
                      .arg(dbPath, QString(encName), QString(encPass), QString(encNotice));
    return !ssh(sql).isNull();
}


QList<QVariantMap> SSHPodkl::selectPasswords(const QString &forUser)
{
    QList<QVariantMap> res;
    if (m_conn.dbType == 'p') {
        QString sql = QString("psql -A -F ',' -d PassHiveDB -c \"SELECT id, encode(name,'base64'), encode(pass,'base64'), encode(notice,'base64') FROM PassHiveDB WHERE \"user\"='%1';\"")
        .arg(forUser);
        QString out = ssh(sql);
        for (const QString &line : out.split('\n')) {
            if (!line.contains(',')) continue;
            auto p = line.split(',');
            if (p.size() < 4) continue;
            QVariantMap row;
            row["id"]     = p[0].toInt();
            row["name"]   = QString::fromUtf8(decrypt(QByteArray::fromBase64(p[1].toLatin1())));
            row["pass"]   = QString::fromUtf8(decrypt(QByteArray::fromBase64(p[2].toLatin1())));
            row["notice"] = QString::fromUtf8(decrypt(QByteArray::fromBase64(p[3].toLatin1())));
            res << row;
        }
        return res;
    }
    // SQLite
    QString dbPath = QString("%1/pass_hive_%2.db").arg(PASSHIVE_ROOT, m_conn.user);
    QString sql = QString("sqlite3 -csv %1 \"SELECT id, hex(name), hex(pass), hex(notice) FROM PassHive;\"").arg(dbPath);
    QString out = ssh(sql);
    for (const QString &line : out.split('\n')) {
        if (!line.contains(',')) continue;
        auto p = line.split(',');
        if (p.size() < 4) continue;
        QVariantMap row;
        row["id"]     = p[0].toInt();
        row["name"]   = QString::fromUtf8(decrypt(QByteArray::fromHex(p[1].toLatin1())));
        row["pass"]   = QString::fromUtf8(decrypt(QByteArray::fromHex(p[2].toLatin1())));
        row["notice"] = QString::fromUtf8(decrypt(QByteArray::fromHex(p[3].toLatin1())));
        res << row;
    }
    return res;
}


bool SSHPodkl::updatePassword(int id, const QString &forUser, const QString &name, const QString &pass, const QString &notice)
{
    QByteArray encName   = encrypt(name.toUtf8()).toBase64();
    QByteArray encPass   = encrypt(pass.toUtf8()).toBase64();
    QByteArray encNotice = encrypt(notice.toUtf8()).toBase64();

    if (m_conn.dbType == 'p') {
        QString sql = QString("psql -d PassHiveDB -c \"UPDATE PassHiveDB SET name=decode('%1','base64'), pass=decode('%2','base64'), notice=decode('%3','base64') WHERE id=%4 AND \"user\"='%5';\"")
        .arg(QString(encName), QString(encPass), QString(encNotice)).arg(id).arg(forUser);
        return !ssh(sql).isNull();
    }
    QString dbPath = QString("%1/pass_hive_%2.db").arg(PASSHIVE_ROOT, m_conn.user);
    QString sql = QString("sqlite3 %1 \"UPDATE PassHive SET name=x'%2', pass=x'%3', notice=x'%4' WHERE id=%5;\"")
                      .arg(dbPath, QString(encName), QString(encPass), QString(encNotice)).arg(id);
    return !ssh(sql).isNull();
}


bool SSHPodkl::deletePassword(int id, const QString &forUser)
{
    if (m_conn.dbType == 'p') {
        QString sql = QString("psql -d PassHiveDB -c \"DELETE FROM PassHiveDB WHERE id=%1 AND \"user\"='%2';\"")
        .arg(id).arg(forUser);
        return !ssh(sql).isNull();
    }
    QString dbPath = QString("%1/pass_hive_%2.db").arg(PASSHIVE_ROOT, m_conn.user);
    QString sql = QString("sqlite3 %1 \"DELETE FROM PassHive WHERE id=%2;\"").arg(dbPath).arg(id);
    return !ssh(sql).isNull();
}


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


bool SSHPodkl::bootstrapNewUser(const QString &login, char dbType)
{
    QString ip   = QInputDialog::getText(nullptr, "IP", "IP сервера:");
    QString user = QInputDialog::getText(nullptr, "SSH user", "Логин:");
    QString pass = QInputDialog::getText(nullptr, "SSH pass", "Пароль:", QLineEdit::Password);
    if (ip.isEmpty() || user.isEmpty() || pass.isEmpty()) return false;

    if (!runCmd("ssh", {QString("%1@%2").arg(user, ip), "echo", "ok"}).contains("ok")) return false;

    QString os; if (!detectOS(os)) return false;
    installPackagesForOS(os);
    createSystemUser(login);

    QByteArray cipher = encrypt(pass.toUtf8());
    QJsonObject e; e["ip"] = ip; e["user"] = user; e["pass"] = QString(cipher.toBase64()); e["dbType"] = QString(dbType);
    m_registry[login] = e; saveRegistry();
    return true;
}


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


QByteArray SSHPodkl::encrypt(const QByteArray &plain)
{
    return m_pm.zakod(QString::fromUtf8(plain), m_key.toUtf8()).toUtf8();
}

QByteArray SSHPodkl::decrypt(const QByteArray &cipher)
{
    return m_pm.raskod(QString::fromUtf8(cipher), m_key.toUtf8()).toUtf8();
}


QString SSHPodkl::runCmd(const QString &program, const QStringList &args, int timeoutMs) const
{
    QProcess p; p.setProgram(program); p.setArguments(args); p.start();
    if (!p.waitForFinished(timeoutMs)) { p.kill(); return {}; }
    return QString::fromUtf8(p.readAllStandardOutput());
}
