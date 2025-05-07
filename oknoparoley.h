#ifndef OKNOPAROLEY_H
#define OKNOPAROLEY_H

#include <QDialog>
#include "dbpoisk.h"
#include "PassManager.h"
#include <QMap>
#include <QListWidgetItem>
#include "sshpodkl.h"

namespace Ui {
class OknoParoley;
}

class OknoParoley : public QDialog
{
    Q_OBJECT

public:
    explicit OknoParoley(QWidget *parent = nullptr);
    ~OknoParoley();

    void obnov_spisok(bool necod = false);
    void sm_r(char r);                     // Переключение режимов
    void primeni();                        // Применить изменения (добавление/редактирование)

public slots:
    void start(QMap<QString, QString> data);
    void smotr(QListWidgetItem *riad);             // Просмотр выбранного пароля
    void vstav();                                  // Начало добавления нового пароля
    void red();
    void udalit();
    void sm_podkl();

    void gen();

signals:
    void obnov();
    void generator();

private:
    Ui::OknoParoley *ui;
    DBPoisk *db;
    PassManager *pm;
    SSHPodkl *ssh;

    QString guiPass;
    char soed;

    QByteArray kluch = "";
    QString login = "";

    QList<QMap<QString, QString>> data;
    QList<QVariantMap> udal_data;


    char rezh; // Режим работы: '.', 'w', 'c', 'a'
};

#endif // OKNOPAROLEY_H
