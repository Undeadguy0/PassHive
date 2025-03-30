#include "configurator.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QDir>
#include <QString>

void Configurator::podkl(QString put) {
    QFile istok(put);

    if (!istok.exists()) {
        qDebug() << "Файл не существует:" << put;
        return;
    }

    if (!istok.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл:" << istok.errorString();
        return;
    }

    qDebug() << "Файл успешно открыт:" << put;

    QTextStream usrs(&istok);


    istok.close();
}


bool Configurator::prov_users() {
    QFile usrs(QDir::currentPath() + "/usrs.txt");

    if (!usrs.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл usrs.txt";
        return false;
    }

    QTextStream potok(&usrs);
    if (potok.atEnd()) {  // Если файл пустой
        potok << "CUSRS = 0";
        usrs.close();
        return false;
    }

    while (!potok.atEnd()) {
        QString liniya = potok.readLine();
        if (liniya.isEmpty()) {
            continue;
        }

        QStringList parts = liniya.split(" ");
        if (parts.size() >= 3) {
            bool ok;
            int znach = parts[2].toInt(&ok);
            if (ok && znach == 0) {
                usrs.close();
                return false;
            }
            usrs.close();
            return true;
        }
    }

    usrs.close();
    return false;
}

std::vector<std::string> pol_polz(){
    std::vector<std::string> a;
    return a;
}


