#ifndef OKNOPAROLEY_H
#define OKNOPAROLEY_H

#include <QDialog>
#include "dbpoisk.h"

namespace Ui {
class OknoParoley;
}

class OknoParoley : public QDialog
{
    Q_OBJECT

public:
    explicit OknoParoley(QWidget *parent = nullptr);
    ~OknoParoley();

public slots:
    void start(QString user, QString pass);

private:
    Ui::OknoParoley *ui;
    DBPoisk *db;
};

#endif // OKNOPAROLEY_H
