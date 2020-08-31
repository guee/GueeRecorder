QT       += core gui multimedia multimediawidgets
linux{
QT += x11extras
}
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

CONFIG += c++11 precompile_header
PRECOMPILED_HEADER = precompile.h
LIBS += -lX11 -lXfixes -lXinerama -lXext
#LIBS += -lavformat -lavcodec -lavutil -lswscale -lX11 -lXext -lXfixes -lasound -ljack -ljacknet -lasound -lpulse -lXi -lXinerama
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
    FormLayerTools.cpp \
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
    VdeoSynthesizer.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Common/FrameRateCalc.h \
    Common/FrameSynchronization.h \
    DialogSelectScreen.h \
    FormLayerTools.h \
    GlScreenSelect.h \
    GlWidgetPreview.h \
    GueeGL/GlDataType.h \
    InputSource/BaseLayer.h \
    InputSource/BaseSource.h \
    InputSource/CameraLayer.h \
    InputSource/CameraSource.h \
    InputSource/PictureLayer.h \
    InputSource/PictureSource.h \
    InputSource/ScreenLayer.h \
    InputSource/ScreenSource.h \
    MediaCodec/EncoderParams.h \
    ShaderProgramPool.h \
    StackedWidgetAddLayer.h \
    VdeoSynthesizer.h \
    mainwindow.h \
    precompile.h

FORMS += \
    DialogSelectScreen.ui \
    FormLayerTools.ui \
    StackedWidgetAddLayer.ui \
    mainwindow.ui
#QMAKE_CXXFLAGS += -fopenmp -march='mips64r5'
QMAKE_CXXFLAGS += -fopenmp
QMAKE_CXXFLAGS_RELEASE += -O3
LIBS += -lgomp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    UiResource/recorder.qrc

DISTFILES += \
    UiResource/Shaders/RgbToYuv.frag \
    UiResource/Shaders/RgbToYuv.vert \
    UiResource/Shaders/SelectScreen.frag \
    UiResource/Shaders/SelectScreen.vert \
    UiResource/Shaders/Test.frag \
    UiResource/Shaders/Test.vert \
    UiResource/Shaders/base.frag \
    UiResource/Shaders/base.vert
