

#pragma once

#include <QtCore>
#include "PassManager.h"

// Корневой каталог для всех данных PassHive
static const QString PASSHIVE_ROOT = "/var/lib/passhive";

struct ConnectionInfo
{
    QString ip;          // IP/host
    QString user;        // SSH‑логин
    QByteArray plainPass;// открытый пароль
    char dbType;         // 'p' (PostgreSQL) | 'l' (SQLite)
};

class SSHPodkl : public QObject
{
    Q_OBJECT
public:
    explicit SSHPodkl(QObject *parent = nullptr);


    bool ensureConnection(const QString &login, char dbType, const QString &accountPassword);

    // вспомогательные операции
    QString ssh(const QString &cmd) const;                                   // произвольная команда
    bool    sftpUpload(const QString &localPath, const QString &remotePath) const; // SCP‑копия

    // CRUD с шифрованием
    bool insertPassword(const QString &forUser,
                        const QString &name,
                        const QString &pass,
                        const QString &notice);
    QList<QVariantMap> selectPasswords(const QString &forUser);

    const ConnectionInfo &connection() const { return m_conn; }

    bool deletePassword(int id, const QString &forUser);
    bool updatePassword(int id, const QString &forUser, const QString &name, const QString &pass, const QString &notice);

private:
    // JSON‑реестр
    bool loadRegistry();
    bool saveRegistry();

    bool bootstrapNewUser(const QString &login, char dbType);
    bool installPackagesForOS(const QString &os);
    bool createSystemUser(const QString &login);


    bool detectOS(QString &outOs);
    bool initDatabase();


    QByteArray encrypt(const QByteArray &plain);
    QByteArray decrypt(const QByteArray &cipher);

private:
    QJsonObject    m_registry;   // users.json в /var/lib/passhive
    ConnectionInfo m_conn;
    PassManager    m_pm;
    QString        m_key;        // accountPassword (ключ шифрования)


    QString runCmd(const QString &program, const QStringList &args, int timeoutMs = 15000) const;
};
