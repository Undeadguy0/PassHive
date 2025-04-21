#include "dbpoisk.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/qsqldatabase.h>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <string>
#include <QString>
#include <QMap>
#include <QtSql/QSqlRecord>


void DBPoisk::init_DB(std::string imia, bool mode) {
    if (!mode) {
        QString connectionName = "conn_" + QString::fromStdString(imia);

        if (QSqlDatabase::contains(connectionName)) {
            corr_db = new QSqlDatabase(QSqlDatabase::database(connectionName));
        } else {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            db.setDatabaseName("data.db");

            if (!db.open()) {
                qDebug() << "ОШИБКА ПОДКЛЮЧЕНИЯ К БД: " << db.lastError().text();
                return;
            }

            corr_db = new QSqlDatabase(db);
        }

        corr_user = QString::fromStdString(imia);

        QSqlQuery dialog(*corr_db);
        QString zapr = "CREATE TABLE IF NOT EXISTS " + corr_user +
                       " (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, pass TEXT, notice TEXT)";

        if (!dialog.exec(zapr)) {
            qDebug() << "Ошибка создания таблицы:" << dialog.lastError().text();
            return;
        }
    }
}


void DBPoisk::vstav_data(QString name, QString pass, QString notice) {
    if (!corr_db->open()) {
        qDebug() << "Не удалось открыть БД!";
        return;
    }

    QSqlQuery dialog(*corr_db);
    QString sql = "INSERT INTO " + corr_user + " (name, pass, notice) VALUES (:name, :pass, :notice)";

    dialog.prepare(sql);
    dialog.bindValue(":name", name);
    dialog.bindValue(":pass", pass);
    dialog.bindValue(":notice", notice);

    if (!dialog.exec()) {
        qDebug() << "Ошибка вставки данных:" << dialog.lastError().text();
    } else {
        qDebug() << "Запись успешно добавлена!";
    }
}

QList<QMap<QString, QString>> DBPoisk::pol_vse() {
    QList<QMap<QString, QString>> result;

    if (!corr_db->open()) {
        qDebug() << "Ошибка открытия БД!";
        return result;
    }

    QSqlQuery dialog(*corr_db);
    QString sql = "SELECT * FROM " + corr_user;

    if (!dialog.exec(sql)) {
        qDebug() << "Ошибка запроса SELECT:" << dialog.lastError().text();
        return result;
    }

    while (dialog.next()) {
        QMap<QString, QString> row;
        for (int i = 0; i < dialog.record().count(); ++i) {
            QString column = dialog.record().fieldName(i);
            QString value = dialog.value(i).toString();
            row[column] = value;
        }
        result.append(row);
    }

    return result;
}

bool DBPoisk::udal(int r){
    if (!corr_db->open()) return false;

    QSqlQuery query(*corr_db);
    QString sql = "DELETE FROM " + corr_user + " WHERE id = :id";
    query.prepare(sql);
    query.bindValue(":id", r);

    if (!query.exec()) {
        qDebug() << "Ошибка удаления:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DBPoisk::redact(int id, const QString& name, const QString& pass, const QString& notice) {
    if (!corr_db->open()) return false;

    QSqlQuery dialog(*corr_db);
    QString sql = "UPDATE " + corr_user + " SET name = :name, pass = :pass, notice = :notice WHERE id = :id";
    dialog.prepare(sql);
    dialog.bindValue(":name", name);
    dialog.bindValue(":pass", pass);
    dialog.bindValue(":notice", notice);
    dialog.bindValue(":id", id);

    if (!dialog.exec()) {
        qDebug() << "Ошибка обновления:" << dialog.lastError().text();
        return false;
    }

    return true;
}


DBPoisk::DBPoisk() {}
