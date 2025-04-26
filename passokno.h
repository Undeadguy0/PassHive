#ifndef PASSOKNO_H
#define PASSOKNO_H

#include <string>
#include <QWidget>
#include <QMap>

namespace Ui {
class PassOkno;
}

class PassOkno : public QWidget
{
    Q_OBJECT

public:
    explicit PassOkno(QWidget *parent = nullptr);
    ~PassOkno();

private:
    Ui::PassOkno *ui;
    void prov();

public slots:
   void vihod();

signals:
    void zakroy();
    void vhod(QMap<QString, QString>);


};

#endif // PASSOKNO_H
