#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <QFile>
#include <QString>
#include <QDir>
#include <vector>
#include <string>


class Configurator{

public:
    QString users = QDir::currentPath() + "/usrs.txt";
    void podkl(QString put);
    bool prov_users();
    std::vector<std::string> pol_polz();

};

#endif // CONFIGURATOR_H
