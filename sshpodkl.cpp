#include "sshpodkl.h"
#include "PassManager.h"
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <QInputDialog>

SSHPodkl::SSHPodkl()
{
    loadConnectsFile();
}

bool SSHPodkl::loadConnectsFile()
{
    QFile file("connects.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Не удалось открыть файл connects.json!";
        return false;
    }
    connectsData = QJsonDocument::fromJson(file.readAll()).object();
    return true;
}

bool SSHPodkl::saveConnectsFile()
{
    QFile file("connects.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Не удалось сохранить файл connects.json!";
        return false;
    }
    return file.write(QJsonDocument(connectsData).toJson()) > 0;
}

bool SSHPodkl::isUserInConnects(const QString& login)
{
    return connectsData.contains(login);
}

QString SSHPodkl::sshExecuteCommand(const QString& command)
{
    qDebug() << "[SSH] Выполняем команду:" << command;

    if (command == "cat /etc/os-release") {
        return "NAME=\"Ubuntu\"\nVERSION=\"22.04.2 LTS\"\n";
    }
    if (command == "uname -s") {
        return "Linux";
    }
    if (command.startsWith("systeminfo")) {
        return "OS Name: Microsoft Windows Server 2019 Standard";
    }

    return "";
}

bool SSHPodkl::sftpUploadFile(const QString& localPath, const QString& remotePath)
{
    qDebug() << "[SFTP] Загружаем файл" << localPath << "в" << remotePath;
    return true;
}

QString SSHPodkl::detectOS()
{
    QString osInfo = sshExecuteCommand("cat /etc/os-release");

    if (!osInfo.isEmpty()) {
        if (osInfo.contains("Ubuntu")) return "Ubuntu";
        if (osInfo.contains("Debian")) return "Debian";
        if (osInfo.contains("Fedora")) return "Fedora";
        if (osInfo.contains("Red Hat")) return "RHEL";
        if (osInfo.contains("CentOS")) return "CentOS";
        if (osInfo.contains("Rocky Linux")) return "Rocky";
        if (osInfo.contains("AlmaLinux")) return "Alma";
        if (osInfo.contains("Oracle Linux")) return "Oracle";
        if (osInfo.contains("Arch Linux")) return "Arch";
        if (osInfo.contains("Alpine")) return "Alpine";
        if (osInfo.contains("SUSE") || osInfo.contains("SLES")) return "SUSE";
    }

    osInfo = sshExecuteCommand("uname -s");
    if (!osInfo.isEmpty()) {
        if (osInfo.contains("FreeBSD")) return "FreeBSD";
        if (osInfo.contains("Linux")) return "Linux";
    }

    osInfo = sshExecuteCommand("systeminfo | findstr /B /C:\"OS Name\"");
    if (!osInfo.isEmpty()) {
        if (osInfo.contains("Windows Server 2022")) return "WindowsServer2022";
        if (osInfo.contains("Windows Server 2019")) return "WindowsServer2019";
        if (osInfo.contains("Windows")) return "Windows";
    }

    return "Unknown";
}

bool SSHPodkl::createDatabaseIfNotExists(const QString& login, char dbType)
{
    if (dbType == 'p') {
        QString checkDbCmd = "psql -lqt | cut -d \"|\" -f 1 | grep -w passhive_" + login + "_db";
        QString result = sshExecuteCommand(checkDbCmd);
        if (result.isEmpty()) {
            QString createDbCmd = "createdb passhive_" + login + "_db";
            if (sshExecuteCommand(createDbCmd).isEmpty()) {
                qDebug() << "Ошибка создания базы данных PostgreSQL!";
                return false;
            }
            qDebug() << "База данных PostgreSQL успешно создана.";
        }
    } else if (dbType == 'l') {
        QString dbPath = "/home/passhive_" + login + "/passhive_" + login + ".db";
        QString checkDbCmd = "test -f " + dbPath;
        QString result = sshExecuteCommand(checkDbCmd);
        if (result.isEmpty()) {
            QString createDbCmd = "sqlite3 " + dbPath + " \"VACUUM;\"";
            if (sshExecuteCommand(createDbCmd).isEmpty()) {
                qDebug() << "Ошибка создания файла базы данных SQLite!";
                return false;
            }
            qDebug() << "Файл базы данных SQLite успешно создан.";
        }
    }
    return true;
}

bool SSHPodkl::createTableIfNotExists(const QString& login, char dbType)
{
    if (dbType == 'p') {
        QString createTableCmd = "psql -U passhive_" + login + " -d passhive_" + login + "_db -c \""
                                                                                         "CREATE TABLE IF NOT EXISTS PassHiveDB ("
                                                                                         "id SERIAL PRIMARY KEY,"
                                                                                         "user TEXT,"
                                                                                         "name TEXT,"
                                                                                         "pass TEXT,"
                                                                                         "notice TEXT"
                                                                                         ");\"";
        if (sshExecuteCommand(createTableCmd).isEmpty()) {
            qDebug() << "Ошибка создания таблицы в PostgreSQL!";
            return false;
        }
    } else if (dbType == 'l') {
        QString dbPath = "/home/passhive_" + login + "/passhive_" + login + ".db";
        QString createTableCmd = "sqlite3 " + dbPath + " \"CREATE TABLE IF NOT EXISTS passhive_" + login + " ("
                                                                                                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                                                                           "name TEXT,"
                                                                                                           "pass TEXT,"
                                                                                                           "notice TEXT"
                                                                                                           ");\"";
        if (sshExecuteCommand(createTableCmd).isEmpty()) {
            qDebug() << "Ошибка создания таблицы в SQLite!";
            return false;
        }
    }
    return true;
}

bool SSHPodkl::connectUser(const QString& login, char dbType)
{
    if (!loadConnectsFile()) return false;

    if (isUserInConnects(login)) {
        qDebug() << "Пользователь найден в connects.json, подключаемся.";
        return true;
    } else {
        qDebug() << "Пользователь не найден. Требуется root доступ.";
        QString rootLogin = QInputDialog::getText(nullptr, "Root Login", "Введите логин root пользователя:");
        QString rootPass = QInputDialog::getText(nullptr, "Root Password", "Введите пароль root:", QLineEdit::Password);

        QString detectedOS = detectOS();

        if (!setupNewUser(login, login, detectedOS)) {
            qDebug() << "Ошибка настройки нового пользователя!";
            return false;
        }

        if (!createDatabaseIfNotExists(login, dbType)) {
            qDebug() << "Ошибка создания базы данных!";
            return false;
        }

        if (!createTableIfNotExists(login, dbType)) {
            qDebug() << "Ошибка создания таблицы!";
            return false;
        }

        PassManager pm;
        QString sol = pm.sozd_sol();
        QByteArray hash = pm.sozd_hash(login, QByteArray::fromHex(sol.toLatin1()));

        QJsonObject newUserInfo;
        newUserInfo["ip"] = "SERVER_IP_PLACEHOLDER";
        newUserInfo["os"] = detectedOS;
        newUserInfo["sol"] = sol;
        newUserInfo["hash"] = QString(hash.toHex());
        newUserInfo["iteracii"] = 150000;

        connectsData[login] = newUserInfo;
        saveConnectsFile();

        qDebug() << "Пользователь успешно создан и добавлен в connects.json!";
        return true;
    }
}
