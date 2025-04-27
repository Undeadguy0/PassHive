#include <QString>
#include <QJsonObject>

class SSHPodkl
{
public:
    SSHPodkl();

    bool connectUser(const QString& login, char dbType);
    bool uploadDatabase(const QString& localPath, const QString& remotePath);
    bool uploadConnects();

private:
    QString ip;
    QString serverOS;
    QString serverLogin;
    QByteArray serverPassHash;

    bool isUserInConnects(const QString& login);
    bool createSystemUser(const QString& rootLogin, const QString& rootPass, const QString& newUser, const QString& password);
    bool installPackage(const QString& os, const QString& packageName);
    bool setupNewUser(const QString& login, const QString& password, const QString& detectedOS);
    bool createDatabaseIfNotExists(const QString& login, char dbType);
    bool createTableIfNotExists(const QString& login, char dbType);

    QString sshExecuteCommand(const QString& command);
    bool sftpUploadFile(const QString& localPath, const QString& remotePath);

    QString detectOS();
    QString getInstallCommand(const QString& os, const QString& packageName);

    QJsonObject connectsData;

    bool loadConnectsFile();
    bool saveConnectsFile();
};
