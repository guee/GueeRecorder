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


COMPILER=$$system($$QMAKE_CC -v 2>&1)

message("$$COMPILER")
message($$QT_ARCH)
#contains 必须和 { 在同一行
if (contains(QT_ARCH, mips64)){
    if(contains(COMPILER, mips64el-linux-gnuabi64)){
        #在UOS for loongson上编译
        LIBS += $$PWD/lib/mips64el-linux-gnuabi64/libx264.so.161
        LIBS += $$PWD/lib/mips64el-linux-gnuabi64/libfaac.so.0
        QMAKE_CXXFLAGS_RELEASE += -O3 -mmsa -march=gs464e -mloongson-ext -mloongson-ext2 -mloongson-mmi -fomit-frame-pointer -fforce-addr -ffast-math -Wall -Wno-maybe-uninitialized -Wshadow -mfp64 -mhard-float -fno-tree-vectorize -fvisibility=hidden
        QMAKE_CFLAGS_RELEASE += -O3 -mmsa -march=gs464e -mloongson-ext -mloongson-ext2 -mloongson-mmi -fomit-frame-pointer -fforce-addr -ffast-math -Wall -Wno-maybe-uninitialized -Wshadow -mfp64 -mhard-float -fno-tree-vectorize -fvisibility=hidden
        QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGIN/lib/mips64el-linux-gnuabi64\'
    }else{
        #在lemote fedora28上编译
        LIBS += $$PWD/lib/mips64el-redhat-linux/libx264.so.161
        LIBS += $$PWD/lib/mips64el-redhat-linux/libfaac.so.0
        QMAKE_CXXFLAGS_RELEASE += -O3 -mmsa -march=mips64r5
        QMAKE_CFLAGS_RELEASE += -O3 -mmsa -march=mips64r5
        QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGIN/lib/mips64el-redhat-linux\'
    }
}else{
    #在UOS for amd64上编译
    contains(COMPILER, x86_64-linux-gnu){
        LIBS += $$PWD/lib/x86_64-linux-gnu/libx264.so.161
        #DEFINES += SYSTEM_X264
        #LIBS += -lx264
        LIBS += $$PWD/lib/x86_64-linux-gnu/libfaac.so.0
        QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGIN/lib/x86_64-linux-gnu\'
    }
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
    MediaCodec/VideoEncoder.h \
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
    Doc/GueeRecorder-ScreenShot-1.png \
    Doc/GueeRecorder-ScreenShot.png \
    Doc/GueeRecorder.sh \
    Doc/Pack/GueeRecorder-uos-amd64/DEBIAN/control \
    Doc/Pack/GueeRecorder-uos-amd64/opt/apps/net.guee.gueerecoder/GueeRecorder \
    Doc/Pack/GueeRecorder-uos-amd64/opt/apps/net.guee.gueerecoder/icon256.png \
    Doc/Pack/GueeRecorder-uos-amd64/opt/apps/net.guee.gueerecoder/libx264.so.161 \
    Doc/pack.sh \
    Install/GueeRecoder-uos-amd64.zip \
    Package/pack-fedora-loongson \
    Package/pack-uos \
    Package/uos-amd64/DEBIAN/control \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/applications/guee-recorder.desktop \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/128x128/apps/GueeRecorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/16x16/apps/GueeRecorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/24x24/apps/GueeRecorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/256x256/apps/GueeRecorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/32x32/apps/GueeRecorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/48x48/apps/GueeRecorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/512x512/apps/GueeRecorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/64x64/apps/GueeRecorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/files/GueeRecorder \
    Package/uos-amd64/opt/apps/net.guee.recorder/files/lib/x86_64-linux-gnu/libfaac.so.0.0.0 \
    Package/uos-amd64/opt/apps/net.guee.recorder/files/lib/x86_64-linux-gnu/libx264.so.161 \
    Package/uos-amd64/opt/apps/net.guee.recorder/info \
    Package/uos-loongson/DEBIAN/control \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/128x128/apps/GueeRecorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/16x16/apps/GueeRecorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/24x24/apps/GueeRecorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/256x256/apps/GueeRecorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/32x32/apps/GueeRecorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/48x48/apps/GueeRecorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/512x512/apps/GueeRecorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/64x64/apps/GueeRecorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/files/GueeRecorder \
    Package/uos-loongson/opt/apps/net.guee.recorder/files/lib/mips64el-linux-gnuabi64/libx264.so.161 \
    Package/uos-loongson/opt/apps/net.guee.recorder/info \
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
    Doc/MainUIDesc.png \
    Doc/Pack/pack-uos-amd64 \
    Doc/Pack/pack-uos-loongson \
    Doc/Pack/GueeRecorder-uos-loongson/DEBIAN/control \
    Doc/Pack/GueeRecorder-uos-loongson/usr/share/applications/guee-recorder.desktop \
    Doc/Pack/GueeRecorder-uos-loongson/usr/bin/GueeRecorder \
    Doc/Pack/GueeRecorder-uos-loongson/usr/lib/mips64el-linux-gnuabi64/libx264.so.161 \
    Doc/Pack/GueeRecorder-uos-loongson/usr/share/icons/hicolor/256x256/apps/guee-recorder.png \
    UiResource/Shaders/x264-mb.frag \
    UiResource/Shaders/x264-mb.vert \
    lib/mips64el-linux-gnuabi64/libx264.so.161 \
    lib/x86_64-linux-gnu/libfaac.so.0.0.0 \
    lib/x86_64-linux-gnu/libx264.so.161 \
    Package/uos-loongson/opt/apps/net.guee.recorder/files/lib/mips64el-linux-gnuabi64/libfaac.so.0.0.0 \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/applications/net.guee.recorder.desktop \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/applications/net.guee.recorder.desktop \
    uos_os_match2020 \
    UiResource/Shaders/RgbToY.frag \
    UiResource/Shaders/RgbToV.frag \
    UiResource/Shaders/RgbToU.frag \
    UiResource/Shaders/RgbToY.vert \
    UiResource/Shaders/RgbToV.vert \
    UiResource/Shaders/RgbToU.vert \
    Package/uos-loongson/opt/apps/net.guee.recorder/files/lib/mips64el-linux-gnuabi64/libfaac.so.0 \
    Package/pack-fedora \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/share/applications/net.guee.recorder.desktop \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/bin/GueeRecorder \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/lib64/libfaac.so.0 \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/lib64/libx264.so.161 \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/share/icons/hicolor/128x128/apps/net.guee.recorder.png \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/share/icons/hicolor/16x16/apps/net.guee.recorder.png \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/share/icons/hicolor/24x24/apps/net.guee.recorder.png \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/share/icons/hicolor/256x256/apps/net.guee.recorder.png \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/share/icons/hicolor/32x32/apps/net.guee.recorder.png \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/share/icons/hicolor/48x48/apps/net.guee.recorder.png \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/share/icons/hicolor/512x512/apps/net.guee.recorder.png \
    Package/fedora-loongson/BUILDROOT/net.guee.recorder-1.0.1-0.mips64el/usr/share/icons/hicolor/64x64/apps/net.guee.recorder.png \
    Package/fedora-loongson/SPECS/GueeRecorder.spec \
    Package/uos-amd64/opt/apps/net.guee.recorder/files/lib/x86_64-linux-gnu/libfaac.so.0 \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/128x128/apps/net.guee.recorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/16x16/apps/net.guee.recorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/24x24/apps/net.guee.recorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/256x256/apps/net.guee.recorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/32x32/apps/net.guee.recorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/48x48/apps/net.guee.recorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/512x512/apps/net.guee.recorder.png \
    Package/uos-loongson/opt/apps/net.guee.recorder/entries/icons/hicolor/64x64/apps/net.guee.recorder.png \
    lib/mips64el-linux-gnuabi64/libfaac.so.0 \
    lib/mips64el-linux-gnuabi64/libx264.so.161 \
    lib/mips64el-redhat-linux/libfaac.so.0 \
    lib/mips64el-redhat-linux/libx264.so.161 \
    lib/x86_64-linux-gnu/libfaac.so.0 \
    lib/x86_64-linux-gnu/libx264.so.161 \
    Doc/GueeRecoder-MainUI.png \
    Doc/GueeRecoder-Main.png \
    Doc/GueeRecoder-MainUI.xcf \
    Doc/LayerTools.xcf \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/applications/net.guee.recorder.desktop \
    Package/uos-amd64/opt/apps/net.guee.recorder/files/GueeRecorder \
    Package/uos-amd64/opt/apps/net.guee.recorder/files/lib/x86_64-linux-gnu/libfaac.so.0 \
    Package/uos-amd64/opt/apps/net.guee.recorder/files/lib/x86_64-linux-gnu/libx264.so.161 \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/128x128/apps/net.guee.recorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/16x16/apps/net.guee.recorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/24x24/apps/net.guee.recorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/256x256/apps/net.guee.recorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/32x32/apps/net.guee.recorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/48x48/apps/net.guee.recorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/512x512/apps/net.guee.recorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/entries/icons/hicolor/64x64/apps/net.guee.recorder.png \
    Package/uos-amd64/opt/apps/net.guee.recorder/info
