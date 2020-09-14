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
    void on_label_Help_linkActivated(const QString &link);

    void on_label_Diary_linkActivated(const QString &link);

    void on_label_18_linkActivated(const QString &link);

    void on_label_20_linkActivated(const QString &link);

    void on_label_21_linkActivated(const QString &link);

    void on_label_Source_linkActivated(const QString &link);

private:
    Ui::FormAboutMe *ui;
};

#endif // FORMABOUTME_H
