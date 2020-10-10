QT       += core gui multimedia multimediawidgets
QT += x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

CONFIG += c++11 precompile_header $(SYS_ARCH)
PRECOMPILED_HEADER = precompile.h
LIBS += -lX11 -lXfixes -lXinerama -lXext -lfaac -lXcomposite

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS GL_GLEXT_PROTOTYPES

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Common/FrameSynchronization.cpp \
    DialogSelectScreen.cpp \
    FormAboutMe.cpp \
    FormAudioRec.cpp \
    FormLayerTools.cpp \
    FormWaitFinish.cpp \
    GlScreenSelect.cpp \
    GlWidgetPreview.cpp \
    InputSource/BaseLayer.cpp \
    InputSource/BaseSource.cpp \
    InputSource/CameraLayer.cpp \
    InputSource/CameraSource.cpp \
    InputSource/PictureLayer.cpp \
    InputSource/PictureSource.cpp \
    InputSource/ScreenLayer.cpp \
    InputSource/ScreenSource.cpp \
    ShaderProgramPool.cpp \
    StackedWidgetAddLayer.cpp \
    main.cpp \
    mainwindow.cpp \
    MediaCodec/VideoEncoder.cpp \
    MediaCodec/MediaStream.cpp \
    MediaCodec/MediaWriter.cpp \
    MediaCodec/MediaWriterFlv.cpp \
    MediaCodec/MediaWriterMp4.cpp \
    MediaCodec/MediaWriterTs.cpp \
    DialogSetting.cpp \
    ButtonWithVolume.cpp \
    MediaCodec/SoundRecorder.cpp \
    FormVolumeAction.cpp \
    Common/FrameTimestamp.cpp \
    Common/FrameRateCalc.cpp \
    VideoSynthesizer.cpp \
    MediaCodec/WaveFile.cpp


HEADERS += \
    Common/FrameRateCalc.h \
    Common/FrameSynchronization.h \
    DialogSelectScreen.h \
    FormAboutMe.h \
    FormAudioRec.h \
    FormLayerTools.h \
    FormWaitFinish.h \
    GlScreenSelect.h \
    GlWidgetPreview.h \
    InputSource/BaseLayer.h \
    InputSource/BaseSource.h \
    InputSource/CameraLayer.h \
    InputSource/CameraSource.h \
    InputSource/PictureLayer.h \
    InputSource/PictureSource.h \
    InputSource/ScreenLayer.h \
    InputSource/ScreenSource.h \
    MediaCodec/x264.h \
    MediaCodec/x264_config.h \
    ShaderProgramPool.h \
    StackedWidgetAddLayer.h \
    mainwindow.h \
    precompile.h \
    MediaCodec/VideoEncoder.h \
    MediaCodec/H264Codec.h \
    MediaCodec/MediaStream.h \
    MediaCodec/MediaWriter.h \
    MediaCodec/MediaWriterFlv.h \
    MediaCodec/MediaWriterMp4.h \
    MediaCodec/MediaWriterTs.h \
    MediaCodec/VideoEncoder.h \
    MediaCodec/mp4struct.h \
    DialogSetting.h \
    ButtonWithVolume.h \
    MediaCodec/SoundRecorder.h \
    FormVolumeAction.h \
    Common/FrameTimestamp.h \
    VideoSynthesizer.h \
    MediaCodec/WaveFile.h

FORMS += \
    DialogSelectScreen.ui \
    FormAboutMe.ui \
    FormAudioRec.ui \
    FormLayerTools.ui \
    FormWaitFinish.ui \
    StackedWidgetAddLayer.ui \
    mainwindow.ui \
    DialogSetting.ui \
    FormVolumeAction.ui

#QMAKE_CXXFLAGS += -fopenmp
QMAKE_CXXFLAGS_RELEASE += -O3
#LIBS += -lgomp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    UiResource/recorder.qrc

DISTFILES += \
    Doc/GueeRecorder-ScreenShot-1.png \
    Doc/GueeRecorder-ScreenShot.png \
    Doc/GueeRecorder.sh \
    Doc/pack.sh \
    Install/GueeRecoder-uos-amd64.zip \
    UiResource/Shaders/RgbToYuv.frag \
    UiResource/Shaders/RgbToYuv.vert \
    UiResource/Shaders/SelectScreen.frag \
    UiResource/Shaders/SelectScreen.vert \
    UiResource/Shaders/Test.frag \
    UiResource/Shaders/Test.vert \
    UiResource/Shaders/base.frag \
    UiResource/Shaders/base.vert \
    README.md \
    LICENSE \
    Doc/logo.psd \
    Doc/icon.psd \
    Doc/GueeRecorder-Help.docx \
    Doc/MainUIDesc.png
