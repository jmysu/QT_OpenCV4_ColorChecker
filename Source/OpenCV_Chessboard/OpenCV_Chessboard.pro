QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


linux {
    message("Linux Opencv4")
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv4
    }
macx {
    message("MacOS Opencv4")
    message(Qt version: $$[QT_VERSION])
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv4
    PKG_CONFIG = /usr/local/bin/pkg-config
    }
win32 {
    message("Win64 Opencv440")
    INCLUDEPATH += D:\opencv440\install\include
    LIBS += -LD:\opencv440\install\x64\mingw\bin \
       -lopencv_core440 \
       -lopencv_highgui440 \
       -lopencv_imgproc440 \
       -lopencv_features2d440 \
       -lopencv_calib3d440 \
       -lopencv_imgcodecs440 \
       -lopencv_video440 \
       -lopencv_videoio440
        }

RESOURCES += \
    resources.qrc

DISTFILES += pic/*.jpg
