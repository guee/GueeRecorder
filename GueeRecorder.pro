QT       += core gui multimedia multimediawidgets
QT += x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

CONFIG += c++11 precompile_header $(SYS_ARCH)
PRECOMPILED_HEADER = precompile.h
LIBS += -lX11 -lXfixes -lXinerama -lXext -lXcomposite


#QMAKE_CXXFLAGS += -fopenmp
#LIBS += -lgomp

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS GL_GLEXT_PROTOTYPES

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#COMPILER=$$system($$QMAKE_CC -v 2>&1)
#if(contains(COMPILER, mips64el-linux-gnuabi64)){
#    DEFINES += DEBIAN
#}
#message($$QT_ARCH)
#contains 必须和 { 在同一行
if (contains(QT_ARCH, mips64)){
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGIN/lib/lisa64\'
}

if (contains(QT_ARCH, x86_64)){
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGIN/lib/amd64\'
}


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
    MediaCodec/x264.161/x264.h \
    MediaCodec/x264.161/x264_config.h \
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
    MediaCodec/mp4struct.h \
    DialogSetting.h \
    ButtonWithVolume.h \
    MediaCodec/SoundRecorder.h \
    FormVolumeAction.h \
    Common/FrameTimestamp.h \
    VideoSynthesizer.h \
    MediaCodec/WaveFile.h \
    MediaCodec/faac.h \
    MediaCodec/faaccfg.h

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



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    UiResource/recorder.qrc

DISTFILES += \
    README.md \
    LICENSE \
    uos_os_match2020 \
    Doc/GueeRecoder-MainUI.png \
    Doc/GueeRecoder-Main.png \
    Doc/GueeRecoder-MainUI.xcf \
    Doc/LayerTools.xcf \
    Doc/GueeRecorder-ScreenShot-1.png \
    Doc/GueeRecorder-ScreenShot.png \
    Doc/GueeRecorder.sh \
    Doc/pack.sh \
    Doc/logo.psd \
    Doc/icon.psd \
    Doc/GueeRecorder-Help.docx \
    Doc/MainUIDesc.png \
    Doc/GueeRecorder-Help.docx \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-2.mips64el/usr/share/applications/net.guee.recorder.desktop \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-2.mips64el/usr/share/icons/hicolor/scalable/apps/net.guee.recorder.svg \
    Package/fedora-loongson/SPECS/GueeRecorder.spec
    Package/uos-loongson/DEBIAN/control \
    Package/uos-loongson/opt/apps/net.guee.recorder/info \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/applications/net.guee.recorder.desktop \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/scalable/apps/net.guee.recorder.svg \
    Package/uos-loongson/opt/apps/net.guee.recorder/files/GueeRecorder \
    Package/uos-amd64/DEBIAN/control \
    Package/uos-amd64/opt/apps/net.guee.recorder/info \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/applications/net.guee.recorder.desktop \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/scalable/apps/net.guee.recorder.svg \
    Package/uos-amd64/opt/apps/net.guee.recorder/files/GueeRecorder \
    lib/amd64/libfaac.so.0 \
    lib/amd64/libx264.so.161 \
    lib/lisa64/3a3000/libfaac.so.0 \
    lib/lisa64/3a3000/libx264.so.161 \
    lib/lisa64/3a4000/libfaac.so.0 \
    lib/lisa64/3a4000/libx264.so.161 \
    lib/lisa64/libstdc++.so.6 \
    Doc/GueeRecorder.sh \
    Doc/pack.sh \
    Package/pack-fedora \
    Package/pack-uos \
    Doc/1.png \
    Doc/2.png \
    Doc/GueeRecoder-Main.png \
    Doc/GueeRecoder-MainUI.png \
    Doc/LayerTools.png \
    Doc/MainUIDesc.png \
    Doc/Option.png \
    Doc/icon.psd \
    Doc/logo.psd \
    Doc/GueeRecoder-MainUI.xcf \
    Doc/LayerTools.xcf \



