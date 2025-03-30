#ifndef PASSOKNO_H
#define PASSOKNO_H

#include <string>
#include <QWidget>

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
public slots:
   std::string auth();
   std::string sozd_polz();
   void vihod();
signals:
    void zakroy();


};

#endif // PASSOKNO_H
