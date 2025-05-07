#ifndef PASSGENERATOR_H
#define PASSGENERATOR_H

#include <QDialog>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

namespace Ui {
class PassGenerator;
}

class PassGenerator : public QDialog
{
    Q_OBJECT

public:
    explicit PassGenerator(QWidget *parent = nullptr);
    ~PassGenerator();

private:
    Ui::PassGenerator *ui;
    bool sim_err = false;
    bool kol_err = false;

    void prov_errs();
    double kalk_ent();

    QChar generateRandomChar();

    QString alph = "ABCDEFGHIJKOPQRSTUvWXYZabcdefijkopqrstuvwxyz";
    QString nums = "0123456789";
    QString specs = "_-;!.*%$#@?";

    QList<QString> *passes;

public slots:
    void start();
    void prov_sim();
    void prov_kol();
    void gen_passes();
    void sohr();

    void copy(QListWidgetItem* item);
};

#endif // PASSGENERATOR_H
