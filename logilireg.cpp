#include "logilireg.h"
#include "ui_logilireg.h"

LogIliReg::LogIliReg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LogIliReg)
{
    ui->setupUi(this);
    ui->text->setWordWrap(true);
    QObject::connect(ui->vhod_button, &QPushButton::clicked, this, &LogIliReg::vhod);
    QObject::connect(ui->reg_button, &QPushButton::clicked, this, &LogIliReg::regist);
}

LogIliReg::~LogIliReg()
{
    delete ui;
}

void LogIliReg::vhod(){
    close();
    emit auth();
}

void LogIliReg::regist(){
    close();
    emit reg();
}
