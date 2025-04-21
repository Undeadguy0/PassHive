#include "mainwindow.h"
#include "passokno.h"
#include "PassManager.h"
#include "logilireg.h"
#include "regokno.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication master(argc, argv);

    LogIliReg start;
    PassOkno pass_okno;
    RegOkno reg;
    MainWindow osn_okno;
    PassManager ps;

    ps.init_hran();

    QObject::connect(&reg, &RegOkno::nazad, &start, &LogIliReg::otkr);
    QObject::connect(&pass_okno, &PassOkno::zakroy, &master, &QApplication::quit);
    QObject::connect(&start, &LogIliReg::auth, &pass_okno, &PassOkno::show);
    QObject::connect(&start, &LogIliReg::reg, &reg, &RegOkno::show);


    if(!ps.ne_pust()){
        start.show();
    }
    else{
        reg.show();
    }

    return master.exec();
}
