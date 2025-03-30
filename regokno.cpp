#include "regokno.h"
#include "ui_regokno.h"
#include "configurator.h"
#include <string>
#include <vector>

RegOkno::RegOkno(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegOkno)
{
    ui->setupUi(this);

}

RegOkno::~RegOkno()
{
    delete ui;
}

std::string RegOkno::regist(){
    std::string login, pass, pass_again;
    show();
    Configurator conf;


}
