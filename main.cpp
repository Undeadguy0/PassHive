#include "mainwindow.h"
#include "passokno.h"
#include "PassManager.h"
#include "logilireg.h"
#include "regokno.h"
#include "oknoparoley.h"
#include <QApplication>
#include "passgenerator.h"

int main(int argc, char *argv[]) {
    QApplication master(argc, argv);

    LogIliReg start;
    PassOkno pass_okno;
    RegOkno reg;
    MainWindow osn_okno;
    PassManager ps;
    OknoParoley ok;
    PassGenerator pg;

    ps.init_hran();

    QObject::connect(&reg, &RegOkno::nazad, &start, &LogIliReg::otkr);
    QObject::connect(&pass_okno, &PassOkno::zakroy, &master, &QApplication::quit);
    QObject::connect(&start, &LogIliReg::auth, &pass_okno, &PassOkno::show);
    QObject::connect(&start, &LogIliReg::reg, &reg, &RegOkno::show);
    QObject::connect(&pass_okno, &PassOkno::vhod, &ok, &OknoParoley::start);
    QObject::connect(&reg, &RegOkno::start_generator, &pg, &PassGenerator::start);
    QObject::connect(&ok, &OknoParoley::generator, &pg, &PassGenerator::start);

    if(!ps.ne_pust()){
        start.show();
    }
    else{
        reg.show();
    }

    return master.exec();
}
