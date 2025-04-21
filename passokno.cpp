#include "passokno.h"
#include "ui_passokno.h"
#include "QPixmap"
#include <QApplication>
#include <QLabel>
#include <string>
#include <QPushButton>


PassOkno::PassOkno(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PassOkno)
{
    ui->setupUi(this);


}

PassOkno::~PassOkno()
{
    delete ui;
}

std::string PassOkno::auth(){
    ui->vvod_knopka->setText("ВВОД");
    ui->vihod_knopka->setText("ВЫЙТИ");
    ui->razg_dialog->setText("Введите логин и пароль вашей учетной записи, бззз...");
    QObject::connect(ui->vihod_knopka, &QPushButton::clicked, this, &PassOkno::vihod);
    show();
    return "ИМБА";
}

void PassOkno::vihod(){
    emit zakroy();
}

std::string PassOkno::sozd_polz(){
    return "";

}
