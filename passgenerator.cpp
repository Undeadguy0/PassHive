#include "passgenerator.h"
#include "ui_passgenerator.h"
#include <QRandomGenerator>
#include <vector>
#include <algorithm>
#include <QClipboard>
#include <cmath>

PassGenerator::PassGenerator(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PassGenerator)
{
    ui->setupUi(this);

    passes = new QList<QString>;

    ui->secret_text->setWordWrap(true);
    ui->ent_text->setWordWrap(true);
    ui->gen_knopka->setEnabled(false);

    QObject::connect(ui->kol_sim, &QTextEdit::textChanged, this, &PassGenerator::prov_sim);
    QObject::connect(ui->gen_knopka, &QPushButton::clicked, this, &PassGenerator::gen_passes);
    QObject::connect(ui->kol_pass, &QTextEdit::textChanged, this, &PassGenerator::prov_kol);
    QObject::connect(ui->pass_spis, &QListWidget::itemDoubleClicked, this, &PassGenerator::copy);
    QObject::connect(ui->sohr_knopka, &QPushButton::clicked, this, &PassGenerator::sohr);
}

double PassGenerator::kalk_ent(){
    return ui->kol_sim->toPlainText().toInt() * std::log2(alph.length() + nums.length() + specs.length());
}

void PassGenerator::sohr() {
    if (passes->isEmpty()) {
        ui->secret_text->setText("Сначала сгенерируйте пароли.");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Сохранить пароли", "", "Текстовые файлы (*.txt)");

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ui->secret_text->setText("Не удалось открыть файл для записи.");
        return;
    }

    QTextStream out(&file);
    for (const QString &pass : *passes) {
        out << pass << "\n";
    }

    file.close();
    ui->secret_text->setText("Пароли успешно сохранены!");
}
void PassGenerator::copy(QListWidgetItem* item){
    if (!item) return;
    QString text = item->text();
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);

    ui->secret_text->setText("Пароль скопирован в буфер обмена!");
}

void PassGenerator::start() {
    show();
}

void PassGenerator::prov_errs() {
    ui->gen_knopka->setEnabled(!(sim_err || kol_err));
}

void PassGenerator::prov_sim() {
    ui->secret_text->setText("");
    QString text = ui->kol_sim->toPlainText();

    if (text.isEmpty()) {
        sim_err = true;
    }

    bool ok;
    int total = text.toInt(&ok);

    if (!ok) {
        ui->secret_text->setText("Ошибка ввода кол-ва символов!");
        sim_err = true;
    } else if (total <= 0) {
        ui->secret_text->setText("Ошибка, символов должно быть больше 0!");
        sim_err = true;
    } else if (total < 8) {
        ui->secret_text->setText("Для большей безопасности рекомендую создавать пароли от 8-ми символов.");
        sim_err = false;
    } else {
        sim_err = false;
    }

    prov_errs();
}

void PassGenerator::prov_kol() {
    ui->secret_text->setText("");
    QString text = ui->kol_pass->toPlainText();

    if (text.isEmpty()) {
        kol_err = true;
    }

    bool ok;
    int total = text.toInt(&ok);

    if (!ok) {
        ui->secret_text->setText("Ошибка ввода кол-ва паролей!");
        kol_err = true;
    } else if (total <= 0) {
        ui->secret_text->setText("Ошибка, паролей должно быть больше 0!");
        kol_err = true;
    } else {
        kol_err = false;
    }

    prov_errs();
}

QChar PassGenerator::generateRandomChar() {
    switch (QRandomGenerator::global()->bounded(3)) {
    case 0: return alph[QRandomGenerator::global()->bounded(alph.length())];
    case 1: return nums[QRandomGenerator::global()->bounded(nums.length())];
    case 2: return specs[QRandomGenerator::global()->bounded(specs.length())];
    default: return '?';
    }
}

void PassGenerator::gen_passes() {
    ui->pass_spis->clear();
    passes->clear();

    QString rec_input = ui->rec_slova->toPlainText();
    QStringList recs = rec_input.split(' ', Qt::SkipEmptyParts);

    int dl = ui->kol_sim->toPlainText().toInt();
    int kol = ui->kol_pass->toPlainText().toInt();

    if (recs.isEmpty()) {
        for (int pass = 0; pass < kol; pass++) {
            QString password = "";
            for (int sim = 0; sim < dl; sim++) {
                password += generateRandomChar();
            }
            passes->append(password);
            ui->pass_spis->addItem(password);
        }
    } else {
        QString rec = recs[0];

        if (rec.length() > dl) {
            ui->secret_text->setText("В предлагаемом слове букв больше, чем в ожидаемом пароле!");
            return;
        }

        for (int i = 0; i < kol; i++) {
            QString total = rec;

            int kol_zam = QRandomGenerator::global()->bounded(rec.length() / 2) + 1;
            std::vector<int> posits;

            for (int time = 0; time < kol_zam; time++) {
                int pos = QRandomGenerator::global()->bounded(rec.length());
                if (std::find(posits.begin(), posits.end(), pos) == posits.end()) {
                    posits.push_back(pos);
                } else {
                    time--;
                }
            }

            for (int pos : posits) {
                total[pos] = generateRandomChar();
            }

            while (total.length() < dl) {
                int insert_pos = QRandomGenerator::global()->bounded(total.length() + 1);
                total.insert(insert_pos, generateRandomChar());
            }

            passes->append(total);
            ui->pass_spis->addItem(total);
            ui->secret_text->setText("Для копиравания в буфер, кликните по паролю дважды.");
        }
    }

    double ent = kalk_ent();
    QString total = "Примерная энтропия каждого пароля: " + QString::number(ent) + " бит" +  "\n";
    if(ent < 40){
        total += "Это слабый пароли, рекомендую увеличить кол-во символов";
        ui->ent_text->setStyleSheet("color: red;");
        }
    else if(ent < 71){
        total += "Это пароли средней надежности";
            ui->ent_text->setStyleSheet("color: yellow;");
        }
    else{
            total += "Это очень надежные пароли!";
            ui->ent_text->setStyleSheet("color: green;");
        }

        ui->ent_text->setText(total);
}

PassGenerator::~PassGenerator() {
    delete passes;
    delete ui;
}
