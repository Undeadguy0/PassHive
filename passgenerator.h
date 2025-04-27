#ifndef PASSGENERATOR_H
#define PASSGENERATOR_H

#include <QDialog>

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
};

#endif // PASSGENERATOR_H
