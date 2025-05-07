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
    connect(ui->pass_gen_knopka, &QPushButton::clicked, this, &OknoParoley::gen);

}

void OknoParoley::gen(){
    emit generator();
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

QList<QVariantMap> SSHPodkl::selectPasswords(const QString &forUser)
{
    QList<QVariantMap> res;
    if (m_conn.dbType == 'p') {
        QString sql = QString("psql -A -F ',' -d PassHiveDB -c \"SELECT id, encode(name,'base64'), encode(pass,'base64'), encode(notice,'base64') FROM PassHiveDB WHERE \"user\"='%1';\"")
        .arg(forUser);
        QString out = ssh(sql);
        for (const QString &line : out.split('\n')) {
            if (!line.contains(',')) continue;
            auto p = line.split(',');
            if (p.size() < 4) continue;
            QVariantMap row;
            row["id"]     = p[0].toInt();
            row["name"]   = QString::fromUtf8(decrypt(QByteArray::fromBase64(p[1].toLatin1())));
            row["pass"]   = QString::fromUtf8(decrypt(QByteArray::fromBase64(p[2].toLatin1())));
            row["notice"] = QString::fromUtf8(decrypt(QByteArray::fromBase64(p[3].toLatin1())));
            res << row;
        }
        return res;
    }

    QString dbPath = QString("%1/pass_hive_%2.db").arg(PASSHIVE_ROOT, m_conn.user);
    QString sql = QString("sqlite3 -csv %1 \"SELECT id, hex(name), hex(pass), hex(notice) FROM PassHive;\"").arg(dbPath);
    QString out = ssh(sql);
    for (const QString &line : out.split('\n')) {
        if (!line.contains(',')) continue;
        auto p = line.split(',');
        if (p.size() < 4) continue;
        QVariantMap row;
        row["id"]     = p[0].toInt();
        row["name"]   = QString::fromUtf8(decrypt(QByteArray::fromHex(p[1].toLatin1())));
        row["pass"]   = QString::fromUtf8(decrypt(QByteArray::fromHex(p[2].toLatin1())));
        row["notice"] = QString::fromUtf8(decrypt(QByteArray::fromHex(p[3].toLatin1())));
        res << row;
    }
    return res;
}

bool SSHPodkl::insertPassword(const QString &forUser, const QString &name, const QString &pass, const QString &notice)
{
    QByteArray encName   = encrypt(name.toUtf8()).toBase64();
    QByteArray encPass   = encrypt(pass.toUtf8()).toBase64();
    QByteArray encNotice = encrypt(notice.toUtf8()).toBase64();

    if (m_conn.dbType == 'p') {
        QString sql = QString("psql -d PassHiveDB -c \"INSERT INTO PassHiveDB (\"user\", name, pass, notice) VALUES ('%1', decode('%2','base64'), decode('%3','base64'), decode('%4','base64'));\"")
        .arg(forUser, QString(encName), QString(encPass), QString(encNotice));
        return !ssh(sql).isNull();
    }

    QString dbPath = QString("%1/pass_hive_%2.db").arg(PASSHIVE_ROOT, m_conn.user);
    QString sql = QString("sqlite3 %1 \"INSERT INTO PassHive (name, pass, notice) VALUES (x'%2', x'%3', x'%4');\"")
                      .arg(dbPath, QString(encName), QString(encPass), QString(encNotice));
    return !ssh(sql).isNull();
}

bool SSHPodkl::updatePassword(int id, const QString &forUser, const QString &name, const QString &pass, const QString &notice)
{
    QByteArray encName   = encrypt(name.toUtf8()).toBase64();
    QByteArray encPass   = encrypt(pass.toUtf8()).toBase64();
    QByteArray encNotice = encrypt(notice.toUtf8()).toBase64();

    if (m_conn.dbType == 'p') {
        QString sql = QString("psql -d PassHiveDB -c \"UPDATE PassHiveDB SET name=decode('%1','base64'), pass=decode('%2','base64'), notice=decode('%3','base64') WHERE id=%4 AND \"user\"='%5';\"")
        .arg(QString(encName), QString(encPass), QString(encNotice)).arg(id).arg(forUser);
        return !ssh(sql).isNull();
    }

    QString dbPath = QString("%1/pass_hive_%2.db").arg(PASSHIVE_ROOT, m_conn.user);
    QString sql = QString("sqlite3 %1 \"UPDATE PassHive SET name=x'%2', pass=x'%3', notice=x'%4' WHERE id=%5;\"")
                      .arg(dbPath, QString(encName), QString(encPass), QString(encNotice)).arg(id);
    return !ssh(sql).isNull();
}

bool SSHPodkl::deletePassword(int id, const QString &forUser)
{
    if (m_conn.dbType == 'p') {
        QString sql = QString("psql -d PassHiveDB -c \"DELETE FROM PassHiveDB WHERE id=%1 AND \"user\"='%2';\"")
        .arg(id).arg(forUser);
        return !ssh(sql).isNull();
    }

    QString dbPath = QString("%1/pass_hive_%2.db").arg(PASSHIVE_ROOT, m_conn.user);
    QString sql = QString("sqlite3 %1 \"DELETE FROM PassHive WHERE id=%2;\"").arg(dbPath).arg(id);
    return !ssh(sql).isNull();
}


OknoParoley::~OknoParoley()
{
    delete ui;
    delete pm;
    delete db;
    delete ssh;
}
