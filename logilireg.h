#ifndef LOGILIREG_H
#define LOGILIREG_H

#include <QDialog>

namespace Ui {
class LogIliReg;
}

class LogIliReg : public QDialog
{
    Q_OBJECT

public:
    explicit LogIliReg(QWidget *parent = nullptr);
    ~LogIliReg();

private:
    Ui::LogIliReg *ui;
signals:
    void auth();
    void reg();

public slots:
    void vhod();
    void regist();
    void otkr();
};

#endif // LOGILIREG_H
