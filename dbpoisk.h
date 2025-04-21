#ifndef DBPOISK_H
#define DBPOISK_H
#include <QString>
#include <QtSql/QSqlDatabase>

class DBPoisk
{

public:
    DBPoisk();

    void init_DB(std::string imia, bool mode=0);
    void vstav_data(QString name, QString pass, QString notice);
    QList<QMap<QString, QString>> pol_vse();
    bool udal(int r);
    bool redact(int id, const QString& name, const QString& pass, const QString& notice);

    QSqlDatabase* corr_db= nullptr;
    QString corr_user = "";
};

#endif // DBPOISK_H
