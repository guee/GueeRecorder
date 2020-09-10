#ifndef FORMLAYERTOOLS_H
#define FORMLAYERTOOLS_H

#include <QWidget>
#include <QScrollBar>
#include "VideoSynthesizer.h"

namespace Ui {
class FormLayerTools;
}

class FormLayerTools : public QWidget
{
    Q_OBJECT

public:
    explicit FormLayerTools(VideoSynthesizer* videoObj, QWidget *parent = nullptr);
    virtual~FormLayerTools() override;
    void enterEvent(QEvent*event) override;
    void setCurrLayer(BaseLayer* layer);
    void setStyleIsLeft(bool isLeft);
private slots:
    void on_pushButtonRemove_clicked();
    void on_pushButtonFullScreen_clicked(bool checked);
    void on_pushButtonAspratio_clicked(bool checked);
    void on_pushButtonMoveUp_clicked();
    void on_pushButtonMoveDown_clicked();
    void on_pushButtonPrev_clicked();
    void on_pushButtonNext_clicked();
    void on_pushButtonHide_clicked();
    void on_spinBoxX_valueChanged(int arg1);
    void on_spinBoxY_valueChanged(int arg1);
    void on_spinBoxW_valueChanged(int arg1);
    void on_spinBoxH_valueChanged(int arg1);
    void on_listLayersSelect_currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
private:
    Ui::FormLayerTools *ui;
    VideoSynthesizer*   m_video = nullptr;
    BaseLayer* m_layer = nullptr;
    int32_t m_posChangeByProg = 0;
    void resetSpinBox();
    void resetLayerList();
    int32_t findLayerItem(BaseLayer* layer);
    void resetButStatus();
signals:
    void removeLayer(BaseLayer* layer);
    void selectLayer(BaseLayer* layer);
    void movedLayer(BaseLayer* layer);
};

#endif // FORMLAYERTOOLS_H
