#ifndef REGOKNO_H
#define REGOKNO_H

#include <string>
#include <QDialog>

namespace Ui {
class RegOkno;
}

class RegOkno : public QDialog
{
    Q_OBJECT

public:
    explicit RegOkno(QWidget *parent = nullptr);
    ~RegOkno();

private:
    Ui::RegOkno *ui;

public slots:
    void regist();
    void check();
signals:
    void nazad();

};

#endif // REGOKNO_H
