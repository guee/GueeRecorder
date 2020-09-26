#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include "VideoSynthesizer.h"
#include "./InputSource/ScreenLayer.h"
#undef Bool
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void moveEvent(QMoveEvent* event) override;
    virtual void closeEvent(QCloseEvent *event) override;
private slots:
    void on_widgetPreview_initGL();

    void on_pushButtonRecStart_clicked();
    void on_pushButtonRecStop_clicked();
    void on_pushButtonRecPause_clicked(bool checked);

    void on_videoSynthesizer_initDone(bool success);
    void on_fpsTimerView_timeout();
    void on_pushButtonClose_clicked();

    void on_pushButtonMinimize_clicked();

    void on_pushButtonScreenSelect_clicked(bool checked);
    void on_pushButtonCameraSelect_clicked(bool checked);
    void on_pushButtonMediaSelect_clicked(bool checked);

    void on_pushButtonMenu_clicked();

private:
    Ui::MainWindow *ui;
    VideoSynthesizer& m_video;
    QTimer* m_fpsTimer = nullptr;
    QPoint m_pressKeyGlobalPos;
    QRect m_pressKeyGeometry;
    QPoint m_pressLeftWndOffset;
    Qt::WindowFrameSection m_hitMain = Qt::NoSection;
    QMenu* m_menu = nullptr;
    void setHitCursor(Qt::WindowFrameSection hit);
    void initMenu();
    static void on_close_step_progress(void* param);
};
#endif // MAINWINDOW_H
