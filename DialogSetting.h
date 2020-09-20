#ifndef DIALOGSETTING_H
#define DIALOGSETTING_H

#include <QDialog>
#include <QScrollBar>

#include "VideoSynthesizer.h"

namespace Ui {
class DialogSetting;
}

class DialogSetting : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetting(QWidget *parent = nullptr);
    ~DialogSetting();

    struct UserSetting
    {
        QString profile;
        QString videoDir;
        QString fileName;
        QString fileType;
    };
    static UserSetting& userSetting() {static UserSetting s; return s;}
    static bool loadProfile();
    static bool saveProfile();
    void resizeEvent(QResizeEvent* event);
    void timerEvent(QTimerEvent* event);
private slots:
    void on_pushButton_Video_clicked(bool checked);
    void on_pushButton_Encode_clicked(bool checked);
    void on_pushButton_Audio_clicked(bool checked);
    void on_pushButton_Save_clicked(bool checked);
    void on_pushButton_Readme_clicked(bool checked);
    void on_pushButton_Resolution_16_9_clicked(bool checked);
    void on_pushButton_Resolution_9_16_clicked(bool checked);
    void on_pushButton_Resolution_4_3_clicked(bool checked);
    void on_pushButton_Resolution_3_4_clicked(bool checked);
    void on_pushButton_Resolution_Userdef_clicked(bool checked);
    void on_horizontalSlider_Resolution_valueChanged(int value);
    void on_spinBox_Width_valueChanged(int arg1);
    void on_spinBox_Height_valueChanged(int arg1);
    void on_horizontalSlider_Fps_valueChanged(int value);
    void on_doubleSpinBox_Fps_valueChanged(double arg1);
    void on_checkBox_RecCursor_clicked(bool checked);
    void on_horizontalSlider_Bps_valueChanged(int value);
    void on_spinBox_Bps_valueChanged(int arg1);
    void on_comboBox_Bps_currentIndexChanged(int index);
    void on_horizontalSlider_Preset_valueChanged(int value);
    void on_doubleSpinBox_Keyframe_valueChanged(double arg1);
    void on_pushButton_Folder_clicked();
    void on_scrollArea_valueChanged(int value);
    void on_resolution_changed();
    void on_radioButton_FileMp4_clicked(bool checked);
    void on_radioButton_FileFlv_clicked(bool checked);
    void on_lineEdit_Filename_editingFinished();
    void on_checkBox_RecAudio_clicked(bool checked);
    void on_horizontalSlider_AudioBitrate_valueChanged(int value);
    void on_spinBox_AudioBitrate_valueChanged(int arg1);
    void on_radioButton_Sample11025_clicked(bool checked);
    void on_radioButton_Sample22050_clicked(bool checked);
    void on_radioButton_Sample44100_clicked(bool checked);
    void on_radioButton_SampleBit16i_clicked(bool checked);
    void on_radioButton_SampleBit32i_clicked(bool checked);
    void on_radioButton_SampleBit32f_clicked(bool checked);
    void on_radioButton_SampleMono_clicked(bool checked);
    void on_radioButton_SampleStereo_clicked(bool checked);
    void on_pushButton_SizeSub_clicked();
    void on_pushButton_SizeAdd_clicked();



    void on_spinBox_QP_valueChanged(int arg1);

    void on_horizontalSlider_QP_valueChanged(int value);

    void on_spinBox_refFrame_valueChanged(int arg1);

    void on_spinBox_bFrame_valueChanged(int arg1);

private:
    Ui::DialogSetting *ui;
    VideoSynthesizer& m_video;
    QScrollBar* m_verticalScrollBar = nullptr;
    QTimer* m_timResolution = nullptr;
    bool m_resolutionChanging = false;
    int checkSizeRatio(int32_t w, int32_t h);
    QSize pixelCountToSize(int32_t c, int32_t w, int32_t h);
    void setResparamToUi(int32_t w, int32_t h, int32_t r = -1);
    void setParamsToUi();
};

#endif // DIALOGSETTING_H
