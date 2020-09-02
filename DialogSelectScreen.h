#ifndef DIALOGSELECTSCREEN_H
#define DIALOGSELECTSCREEN_H

#include <QDialog>
#include "InputSource/ScreenLayer.h"
#undef Bool

namespace Ui {
class DialogSelectScreen;
}

class DialogSelectScreen : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectScreen(QWidget *parent = nullptr);
    ~DialogSelectScreen();
    Ui::DialogSelectScreen *ui;

    ScreenLayer::Option& option();
private:
    QRect m_mainOldGeometry;


private slots:
    void on_widget_selected(bool cancel);
    void on_widget_editing(bool ready, const QRect& box);
    void on_toolButtonOK_clicked();
    void on_toolButtonCancel_clicked();
    void on_toolButtonSave_clicked();
};

#endif // DIALOGSELECTSCREEN_H
