#include "mainwindow.h"
#include "passokno.h"
#include "logilireg.h"
#include "regokno.h"
#include "configurator.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication master(argc, argv);

    Configurator conf;
    LogIliReg start;
    PassOkno pass_okno;
    RegOkno reg;
    MainWindow osn_okno;


    QObject::connect(&pass_okno, &PassOkno::zakroy, &master, &QApplication::quit);
    QObject::connect(&start, &LogIliReg::auth, &pass_okno, &PassOkno::show);
    QObject::connect(&start, &LogIliReg::reg, &reg, &RegOkno::show);


    if (!conf.prov_users()) {
        reg.show();
    } else {
        start.show();
    }

    return master.exec();
}
