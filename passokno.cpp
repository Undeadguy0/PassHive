#include "passokno.h"
#include "ui_passokno.h"
#include "QPixmap"
#include <QApplication>
#include <QLabel>
#include "PassManager.h"
#include <QPushButton>
#include <QMap>


PassOkno::PassOkno(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PassOkno)
{
    ui->setupUi(this);

    ui->vvod_knopka->setText("ВВОД");
    ui->vihod_knopka->setText("ВЫЙТИ");
    ui->razg_dialog->setText("Введите логин и пароль вашей учетной записи, бззз...");
    QObject::connect(ui->vihod_knopka, &QPushButton::clicked, this, &PassOkno::vihod);
    QObject::connect(ui->vvod_knopka, &QPushButton::clicked, this, &PassOkno::prov);


}

void PassOkno::prov(){
    PassManager pm;
    QString login = ui->login_dialog->toPlainText();
    QString pass = ui->parol_dialog->toPlainText();

    if(pm.prov_pass(login, pass)){
        QMap<QString, QString> total;
        total["user"] = login;
        total["pass"] = pass;
        emit vhod(total);
        close();
    }

    ui->razg_dialog->setText("Бзз.. Ошибка в пароле или логине");
}

PassOkno::~PassOkno()
{
    delete ui;
}


void PassOkno::vihod(){
    emit zakroy();
}

