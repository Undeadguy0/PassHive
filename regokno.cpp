#include "regokno.h"
#include "ui_regokno.h"
#include "PassManager.h"
#include <QThread>

#include <QString>
RegOkno::RegOkno(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegOkno)
{
    ui->setupUi(this);
    ui->osnov_dialog->setAlignment(Qt::AlignCenter);
    QObject::connect(ui->regist_knopka, &QPushButton::clicked, this, &RegOkno::check);
    QObject::connect(ui->gen_knopka, &QPushButton::clicked, this, &RegOkno::generator);

}

RegOkno::~RegOkno()
{
    delete ui;
}
void RegOkno::regist(){
    show();
}

void RegOkno::generator(){
    emit start_generator();
}

void RegOkno::check(){
    PassManager ps;

    auto login = ui->log_browser->toPlainText();
    auto p1 = ui->pass_browser->toPlainText();
    auto p2 = ui->pass_again_browser->toPlainText();

    if(ps.sush(login)){
        ui->osnov_dialog->setText("Такой логин уже использован");
        return;
    }
    if(p1 != p2){
        ui->osnov_dialog->setText("Пароли не совпадают!");
        return;
    }

    ps.reg_polz(login, p1);
    ui->osnov_dialog->setText("Регистрация прошла успешно!");

    QThread::sleep(2);
    emit nazad();
    close();

}
