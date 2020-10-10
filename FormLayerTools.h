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
    explicit FormLayerTools(QWidget *parent = nullptr);
    virtual~FormLayerTools() override;
    void enterEvent(QEvent*event) override;
    void setCurrLayer(BaseLayer* layer);
    void setStyleIsLeft(bool isLeft);
    void refreshLayers(VideoSynthesizer* videoObj);
    bool windowIsPeg();
private slots:
    void on_pushButtonRemove_clicked();
    void on_pushButtonFullScreen_clicked(bool checked);
    void on_pushButtonAspratio_clicked(bool checked);
    void on_pushButtonMoveUp_clicked();
    void on_pushButtonMoveDown_clicked();

    void on_spinBoxX_valueChanged(int arg1);
    void on_spinBoxY_valueChanged(int arg1);
    void on_spinBoxW_valueChanged(int arg1);
    void on_spinBoxH_valueChanged(int arg1);
    void on_listLayersSelect_currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_pushButtonDing_clicked(bool checked);

    void on_horizontalSliderHue_valueChanged(int value);

    void on_checkBoxTinting_clicked(bool checked);

    void on_horizontalSliderSaturability_valueChanged(int value);

    void on_horizontalSliderLuminance_valueChanged(int value);

    void on_horizontalSliderContrast_valueChanged(int value);

    void on_horizontalSliderTransparence_valueChanged(int value);

public slots:
    void on_layerAdded(BaseLayer* layer);
    void on_layerRemoved(BaseLayer* layer);
    void on_selectLayer(BaseLayer* layer);
    void on_layerMoved(BaseLayer* layer);
private:
    Ui::FormLayerTools *ui;
    VideoSynthesizer*   m_video = nullptr;
    BaseLayer* m_layer = nullptr;
    int32_t m_posChangeByProg = 0;

    int32_t findLayerItem(BaseLayer* layer);
    void resetButStatus();
signals:
    void selectLayer(BaseLayer* layer);
};

#endif // FORMLAYERTOOLS_H
