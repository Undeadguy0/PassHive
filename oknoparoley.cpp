#include "oknoparoley.h"
#include "ui_oknoparoley.h"
#include <QMap>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include "sshpodkl.h"


OknoParoley::OknoParoley(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OknoParoley)
{
    ui->setupUi(this);

    db = new DBPoisk();
    pm = new PassManager();
    ssh = new SSHPodkl();

    ui->conn_spisok->addItem(" Локальное хранилище");
    ui->conn_spisok->addItem(" Удаленное SQLite");
    ui->conn_spisok->addItem(" Удаленное Postgres");

    sm_r('.');

    soed = 'l'; // 'l' - локально, 'L' - SQLite, 'P' - Postgres

    connect(ui->spis_paroley, &QListWidget::itemClicked, this, &OknoParoley::smotr);
    connect(ui->dobav_knopka, &QPushButton::clicked, this, &OknoParoley::vstav);
    connect(ui->prim_knopka, &QPushButton::clicked, this, &OknoParoley::primeni);
    connect(ui->izm_knopka, &QPushButton::clicked, this, &OknoParoley::red);
    connect(ui->del_knopka, &QPushButton::clicked, this, &OknoParoley::udalit);
    connect(ui->remote_knopka, &QPushButton::clicked, this, &OknoParoley::sm_podkl);

}

void OknoParoley::sm_podkl(){
    if(ui->conn_spisok->currentIndex() == 0){
        soed = 'l';
    }
    else if(ui->conn_spisok->currentIndex() == 1){
        soed = 'L';
        if (!ssh->ensureConnection(login, 'l', guiPass)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к SQLite‑серверу");
            soed = 'l';
        }
        udal_data = ssh->selectPasswords(login);
    }
    else{
        soed = 'R';
        if (!ssh->ensureConnection(login, 'p', guiPass)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к PostgreSQL‑серверу");
            soed = 'l';
        }
        udal_data = ssh->selectPasswords(login);
    }

    obnov_spisok();
}
void OknoParoley::sm_r(char r){
    switch(r){
    case '.':
        ui->del_knopka->setEnabled(false);
        ui->izm_knopka->setEnabled(false);
        ui->izm_knopka->setText("Изменить");
        ui->del_knopka->setText("Удалить");
        ui->prim_knopka->setEnabled(false);
        ui->dobav_knopka->setEnabled(true);
        ui->name_edit->setReadOnly(true);
        ui->pass_edit->setReadOnly(true);
        ui->notice_edit->setReadOnly(true);
        rezh = '.';
        break;

    case 'c':
        sm_r('.');
        ui->izm_knopka->setText("Отменить");
        ui->prim_knopka->setEnabled(true);
        ui->name_edit->setReadOnly(false);
        ui->pass_edit->setReadOnly(false);
        ui->notice_edit->setReadOnly(false);
        rezh = 'c';
        break;

    case 'w':
        sm_r('.');
        ui->izm_knopka->setEnabled(true);
        ui->del_knopka->setEnabled(true);
        rezh = 'w';
        break;

    case 'a':
        sm_r('.');
        ui->del_knopka->setText("Очистить");
        ui->del_knopka->setEnabled(true);
        ui->izm_knopka->setText("Отменить");
        ui->izm_knopka->setEnabled(true);
        ui->prim_knopka->setEnabled(true);
        ui->dobav_knopka->setDisabled(true);
        ui->name_edit->setReadOnly(false);
        ui->pass_edit->setReadOnly(false);
        ui->notice_edit->setReadOnly(false);
        rezh = 'a';
        break;

    default:
        rezh = '.';
        break;
    }
}

void OknoParoley::start(QMap<QString, QString> data){
    login   = data["user"];
    guiPass = data["pass"];
    db->init_DB(login.toStdString(), 0);
    kluch   = pm->sozd_kl(guiPass);
    this->data = db->pol_vse();

    sm_r('.');
    show();
    obnov_spisok(false);
}

void OknoParoley::smotr(QListWidgetItem *riad){
    sm_r('w');
    int ind = ui->spis_paroley->row(riad);

    ui->name_edit->setText(pm->raskod(data[ind]["name"], kluch));
    ui->pass_edit->setText(pm->raskod(data[ind]["pass"], kluch));
    ui->notice_edit->setText(pm->raskod(data[ind]["notice"], kluch));
}

void OknoParoley::vstav(){
    ui->name_edit->clear();
    ui->pass_edit->clear();
    ui->notice_edit->clear();
    sm_r('a');
}

void OknoParoley::primeni() {
    QString name   = ui->name_edit->toPlainText();
    QString pass   = ui->pass_edit->toPlainText();
    QString notice = ui->notice_edit->toPlainText();

    if (name.isEmpty() || pass.isEmpty()) {
        qDebug() << "Имя и пароль не могут быть пустыми!";
        return;
    }

    if (soed == 'l') {
        // ЛОКАЛЬНОЕ ХРАНИЛИЩЕ
        QString zakod_name   = pm->zakod(name, kluch);
        QString zakod_pass   = pm->zakod(pass, kluch);
        QString zakod_notice = pm->zakod(notice, kluch);

        if (rezh == 'a') {
            db->vstav_data(zakod_name, zakod_pass, zakod_notice);
        }
        else if (rezh == 'c') {
            int ind = ui->spis_paroley->currentRow();
            if (ind < 0 || ind >= data.size()) {
                qDebug() << "Неверный индекс для редактирования!";
                return;
            }

            int id = data[ind]["id"].toInt();
            if (!db->redact(id, zakod_name, zakod_pass, zakod_notice)) {
                qDebug() << "Ошибка редактирования записи в БД!";
                return;
            }
        }

        data = db->pol_vse();
    } else {
        // УДАЛЁННОЕ ХРАНИЛИЩЕ
        if (rezh == 'a') {
            if (!ssh->insertPassword(login, name, pass, notice)) {
                qDebug() << "Ошибка при добавлении пароля на сервер.";
                return;
            }
        }
        else if (rezh == 'c') {
            int ind = ui->spis_paroley->currentRow();
            if (ind < 0 || ind >= udal_data.size()) {
                qDebug() << "Неверный индекс для редактирования!";
                return;
            }

            int id = udal_data[ind]["id"].toInt();
            if (!ssh->updatePassword(id, login, name, pass, notice)) {
                qDebug() << "Ошибка обновления записи на сервере!";
                return;
            }
        }

        udal_data = ssh->selectPasswords(login);
    }

    sm_r('.');
    obnov_spisok(false);
}

void OknoParoley::obnov_spisok(bool necod) {
    ui->spis_paroley->clear();

    if (soed == 'l') {
        for (const auto &riad : data) {
            QString name = riad["name"];
            if (!necod) {
                name = pm->raskod(name, kluch);
            }
            ui->spis_paroley->addItem(name);
        }
    } else {
        for (const auto &row : udal_data) {
            QString name = row["name"].toString();
            ui->spis_paroley->addItem(name);
        }
    }
}


void OknoParoley::red() {
    if (rezh == 'w') {
        sm_r('c');
    } else {
        sm_r('.');
    }
}

void OknoParoley::udalit() {
    if (rezh != 'w') {
        qDebug() << "Удаление невозможно: не выбран элемент.";
        return;
    }

    int ind = ui->spis_paroley->currentRow();
    if (ind < 0 || (soed == 'l' ? ind >= data.size() : ind >= udal_data.size())) {
        qDebug() << "Некорректный индекс записи для удаления.";
        return;
    }

    QString name;
    int id;

    if (soed == 'l') {
        name = pm->raskod(data[ind]["name"], kluch);
        id = data[ind]["id"].toInt();
    } else {
        name = udal_data[ind]["name"].toString();
        id = udal_data[ind]["id"].toInt();
    }

    // Запрашиваем подтверждение удаления
    bool ok;
    QString input = QInputDialog::getText(this,
                                          "Подтверждение удаления",
                                          QString("Введите название пароля \"%1\" для подтверждения удаления:").arg(name),
                                          QLineEdit::Normal,
                                          "",
                                          &ok);

    if (!ok || input.trimmed() != name) {
        qDebug() << "Подтверждение удаления не пройдено.";
        QMessageBox::warning(this, "Удаление отменено", "Имя не совпадает. Удаление прервано.");
        return;
    }

    // Удаляем запись
    bool success = false;
    if (soed == 'l') {
        success = db->udal(id);
        if (success) data = db->pol_vse();
    } else {
        success = ssh->deletePassword(id, login);
        if (success) udal_data = ssh->selectPasswords(login);
    }

    if (!success) {
        QMessageBox::critical(this, "Ошибка", "Ошибка удаления записи.");
        return;
    }

    sm_r('.');
    obnov_spisok(false);
}


OknoParoley::~OknoParoley()
{
    delete ui;
    delete pm;
    delete db;
    delete ssh;
}
