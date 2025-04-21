#ifndef PASSMANAGER_H
#define PASSMANAGER_H

#include <QString>
#include <QJsonObject>

class PassManager{

    const int ITERACII = 150000;
    const int SOL = 32;
    QString sozd_sol();
    QByteArray sozd_hash(QString pass, QByteArray sol);
    QJsonObject zag_data();
    bool sohr_data(QJsonObject data);

public:
    bool init_hran();
    bool reg_polz(QString login, QString pass);
    bool prov_pass(QString login, QString pass);
    bool sush(QString login);
    bool ne_pust();

    QByteArray sozd_kl(QString pass);
    QString zakod(QString data, QByteArray kl);
    QString raskod(QString kod_data, QByteArray kl);
};

#endif // PASSMANAGER_H
