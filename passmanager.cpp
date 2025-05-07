#include "PassManager.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include "qaesencryption.h"




QString PassManager::sozd_sol(){
    QByteArray sol(SOL, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(sol.data()), sol.size() / sizeof(quint32));
    return QString(sol.toHex());
}

QByteArray PassManager::sozd_hash(QString pass, QByteArray sol){
    QByteArray hash = pass.toUtf8();
    for (int i = 0; i < ITERACII; ++i) {
        hash = QCryptographicHash::hash(sol + hash, QCryptographicHash::Blake2b_512);
    }
    return hash;
}

QJsonObject PassManager::zag_data() {
    QFile osech("users.json");
    if (!osech.open(QIODevice::ReadOnly)) {
        qDebug() << "Ошибка открытия файла!";
        return QJsonObject();
    }
    return QJsonDocument::fromJson(osech.readAll()).object();
}

bool PassManager::sohr_data(QJsonObject data) {
    QFile osech("users.json");
    if (!osech.open(QIODevice::WriteOnly)) {
        qDebug() << "Ошибка при сохранении!";
        return false;
    }
    return osech.write(QJsonDocument(data).toJson()) > 0;
}

bool PassManager::init_hran(){
    QFile osech("users.json");

    if(!osech.exists()){
        QJsonObject start_data;
        start_data["polzovateli"] = QJsonArray();
        return sohr_data(start_data);
    }

    return true;
}
bool PassManager::reg_polz(QString login, QString pass) {
    QJsonObject data = zag_data();
    QJsonArray spisok_polz = data["polzovateli"].toArray();

    for (const QJsonValue &znachenie : spisok_polz) {
        if (znachenie.toObject()["login"] == login) {
            qDebug() << "Пользователь существует!";
            return false;
        }
    }

    QString sol = sozd_sol();
    QByteArray hash = sozd_hash(pass, QByteArray::fromHex(sol.toLatin1()));

    QJsonObject noviyPolzovatel;
    noviyPolzovatel["login"] = login;
    noviyPolzovatel["sol"] = sol;
    noviyPolzovatel["hash"] = QString(hash.toHex());
    noviyPolzovatel["iteracii"] = ITERACII;
    noviyPolzovatel["BD"] = "x";

    spisok_polz.append(noviyPolzovatel);
    data["polzovateli"] = spisok_polz;

    return sohr_data(data);
}

bool PassManager::prov_pass(QString login, QString pass) {
    QJsonObject data = zag_data();
    QJsonArray spisok_polz = data["polzovateli"].toArray();

    for (const QJsonValue &znachenie : spisok_polz) {
        QJsonObject polz = znachenie.toObject();
        if (polz["login"].toString() == login) {
            QByteArray sol = QByteArray::fromHex(polz["sol"].toString().toLatin1());
            QByteArray hranimiy_hash = QByteArray::fromHex(polz["hash"].toString().toLatin1());
            QByteArray prover_hash = sozd_hash(pass, sol);
            return (prover_hash == hranimiy_hash);
        }
    }
    return false;
}

bool PassManager::sush(QString login) {
    QJsonObject dannye = zag_data();
    QJsonArray spisok_polz = dannye["polzovateli"].toArray();

    for (const QJsonValue &znachenie : spisok_polz) {
        if (znachenie.toObject()["login"] == login) {
            return true;
        }
    }
    return false;
}

bool PassManager::ne_pust(){
    QJsonObject a = zag_data();
    QJsonArray b = a["polzovateli"].toArray();
    return b.isEmpty();
}

QByteArray PassManager::sozd_kl(QString pass){
    QByteArray hash = QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256);
    return hash;
}

QString PassManager::zakod(QString data, QByteArray kl){
    QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
    QByteArray iv(16,0);
    QRandomGenerator::global()->generate(iv.begin(), iv.end());

    QByteArray encrypted = encryption.encode(data.toUtf8(), kl, iv);
    return QString((iv + encrypted).toBase64());
}

QString PassManager::raskod(QString zag_data, QByteArray kl)
{
    if (zag_data.isEmpty())
        return {};

    QAESEncryption aes(QAESEncryption::AES_256, QAESEncryption::CBC);
    QByteArray raw = QByteArray::fromBase64(zag_data.toUtf8());
    if (raw.size() < 16)
        return QStringLiteral("[Ошибка данных]");

    QByteArray iv        = raw.left(16);
    QByteArray encrypted = raw.mid(16);

    QByteArray decrypted = aes.decode(encrypted, kl, iv);

    decrypted = aes.removePadding(decrypted);
    QString result = QString::fromUtf8(decrypted);

    return result;
}


