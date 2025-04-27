#include "passgenerator.h"
#include "ui_passgenerator.h"

PassGenerator::PassGenerator(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PassGenerator)
{
    ui->setupUi(this);
}

PassGenerator::~PassGenerator()
{
    delete ui;
}
