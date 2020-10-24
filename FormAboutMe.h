#ifndef FORMABOUTME_H
#define FORMABOUTME_H

#include <QWidget>

namespace Ui {
class FormAboutMe;
}

class FormAboutMe : public QWidget
{
    Q_OBJECT

public:
    explicit FormAboutMe(QWidget *parent = nullptr);
    ~FormAboutMe();

private slots:


private:
    Ui::FormAboutMe *ui;
};

#endif // FORMABOUTME_H
