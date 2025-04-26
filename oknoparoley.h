#ifndef OKNOPAROLEY_H
#define OKNOPAROLEY_H

#include <QDialog>
#include "dbpoisk.h"
#include "PassManager.h"
#include <QMap>
#include <QListWidgetItem>

namespace Ui {
class OknoParoley;
}

class OknoParoley : public QDialog
{
    Q_OBJECT

public:
    explicit OknoParoley(QWidget *parent = nullptr);
    ~OknoParoley();

    void obnov_spisok(bool necod = false); // Обновить список паролей
    void sm_r(char r);                     // Переключение режимов
    void primeni();                        // Применить изменения (добавление/редактирование)

public slots:
    void start(QMap<QString, QString> data);      // Запуск окна с передачей данных
    void smotr(QListWidgetItem *riad);             // Просмотр выбранного пароля
    void vstav();                                  // Начало добавления нового пароля
    void red();
    void udalit();

signals:
    void obnov(); // Сигнал на обновление

private:
    Ui::OknoParoley *ui;
    DBPoisk *db;
    PassManager *pm;

    QByteArray kluch = "";
    QList<QMap<QString, QString>> data;
    char rezh; // Режим работы: '.', 'w', 'c', 'a'
};

#endif // OKNOPAROLEY_H
