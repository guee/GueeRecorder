#ifndef FORMWAITFINISH_H
#define FORMWAITFINISH_H

#include <QWidget>

namespace Ui {
class FormWaitFinish;
}

class FormWaitFinish : public QWidget
{
    Q_OBJECT

public:
    explicit FormWaitFinish(QWidget *parent = nullptr);
    ~FormWaitFinish();
    void setDispText(const QString& str);
private:
    Ui::FormWaitFinish *ui;
};

#endif // FORMWAITFINISH_H
