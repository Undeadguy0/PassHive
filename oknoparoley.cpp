#include "oknoparoley.h"
#include "ui_oknoparoley.h"
#include "dbpoisk.h"

OknoParoley::OknoParoley(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OknoParoley)
{
    ui->setupUi(this);
}

void OknoParoley::start(QString user, QString pass){
    db->init_DB(user.toStdString(), 0);


}
OknoParoley::~OknoParoley()
{
    delete ui;
}
