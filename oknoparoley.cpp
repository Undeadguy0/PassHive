#include "oknoparoley.h"
#include "ui_oknoparoley.h"
#include <QMap>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>


OknoParoley::OknoParoley(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OknoParoley)
{
    ui->setupUi(this);

    db = new DBPoisk();
    pm = new PassManager();

    sm_r('.');

    connect(ui->spis_paroley, &QListWidget::itemClicked, this, &OknoParoley::smotr);
    connect(ui->dobav_knopka, &QPushButton::clicked, this, &OknoParoley::vstav);
    connect(ui->prim_knopka, &QPushButton::clicked, this, &OknoParoley::primeni);
    connect(ui->izm_knopka, &QPushButton::clicked, this, &OknoParoley::red);
    connect(ui->del_knopka, &QPushButton::clicked, this, &OknoParoley::udalit);

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
    }
}

void OknoParoley::start(QMap<QString, QString> data){
    db->init_DB(data["user"].toStdString(), 0);
    kluch = pm->sozd_kl(data["pass"]);
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
    QString name = ui->name_edit->toPlainText();   // text(), а не toPlainText()
    QString pass = ui->pass_edit->toPlainText();
    QString notice = ui->notice_edit->toPlainText();

    if (name.isEmpty() || pass.isEmpty()) {
        qDebug() << "Имя и пароль не могут быть пустыми!";
        return;
    }

    QString zakod_name = pm->zakod(name, kluch);
    QString zakod_pass = pm->zakod(pass, kluch);
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

    // После добавления или редактирования перезагружаем всё
    data = db->pol_vse();
    sm_r('.');
    obnov_spisok(false);
}

void OknoParoley::obnov_spisok(bool necod){
    ui->spis_paroley->clear();

    for (auto &riad : data) {
        QString name = riad["name"];
        if (!necod) {
            name = pm->raskod(name, kluch);
        }
        ui->spis_paroley->addItem(name);
    }
}

void OknoParoley::red() {
    if (rezh == 'w') {
        sm_r('c'); // разрешаем редактирование если в режиме просмотра
    } else {
        sm_r('.'); // отмена редактирования
    }
}

void OknoParoley::udalit() {
    if (rezh != 'w') {
        qDebug() << "Удаление невозможно: не выбран элемент.";
        return;
    }

    int ind = ui->spis_paroley->currentRow();
    if (ind < 0 || ind >= data.size()) {
        qDebug() << "Некорректный индекс записи для удаления.";
        return;
    }

    QString name = pm->raskod(data[ind]["name"], kluch); // Берем название записи, которую собираемся удалить

    // Запрашиваем подтверждение имени
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
    int id = data[ind]["id"].toInt();
    if (!db->udal(id)) {
        qDebug() << "Ошибка удаления из БД.";
        return;
    }

    // Обновляем список
    data = db->pol_vse();
    sm_r('.');
    obnov_spisok(false);
}


OknoParoley::~OknoParoley()
{
    delete ui;
    delete pm;
    delete db;
}
