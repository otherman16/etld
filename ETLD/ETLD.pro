TEMPLATE = app
CONFIG += console c++11 threads
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(debug, debug|release):  DEFINES += DEBUG

SOURCES += \
        etld/etld.cpp \
        etld/etldclassifier.cpp \
        etld/etlddetector.cpp \
        etld/etldframe.cpp \
        etld/etldimage.cpp \
        etld/etldintegrator.cpp \
        etld/etldlearning.cpp \
        etld/etldmodel.cpp \
        etld/etldsimpleimage.cpp \
        etld/etldtracker.cpp \
        etld/my_fast.cpp \
        main.cpp

HEADERS += \
    etld/etld.h \
    etld/etld_global.h \
    etld/etldclassifier.h \
    etld/etlddetector.h \
    etld/etldframe.h \
    etld/etldimage.h \
    etld/etldintegrator.h \
    etld/etldlearning.h \
    etld/etldmodel.h \
    etld/etldsimpleimage.h \
    etld/etldtracker.h \
    etld/my_fast.h

#---------------------------------------------------------------------------
#----- Platform Version
#---------------------------------------------------------------------------
win32:PLATFORM_VER = 64
win32:PLATFORM = win$$PLATFORM_VER
unix:PLATFORM = unix$$PLATFORM_VER
#---------------------------------------------------------------------------
#----- OpenCV
#---------------------------------------------------------------------------
win32 {
OPENCV_VER = 410
OPENCV_SDK = $$(OPENCV_SDK)
INCLUDEPATH += $$OPENCV_SDK/include
LIBS += -L$$OPENCV_SDK/x$$PLATFORM_VER/mingw/bin -lopencv_core$${OPENCV_VER}
LIBS += -L$$OPENCV_SDK/x$$PLATFORM_VER/mingw/bin -lopencv_imgproc$${OPENCV_VER}
LIBS += -L$$OPENCV_SDK/x$$PLATFORM_VER/mingw/bin -lopencv_imgcodecs$${OPENCV_VER}
LIBS += -L$$OPENCV_SDK/x$$PLATFORM_VER/mingw/bin -lopencv_videoio$${OPENCV_VER}
LIBS += -L$$OPENCV_SDK/x$$PLATFORM_VER/mingw/bin -lopencv_highgui$${OPENCV_VER}
LIBS += -L$$OPENCV_SDK/x$$PLATFORM_VER/mingw/bin -lopencv_tracking$${OPENCV_VER}
}
unix {
LIBS += \
-lopencv_core \
-lopencv_imgproc \
-lopencv_imgcodecs \
-lopencv_videoio \
-lopencv_highgui \
-lopencv_tracking
}
#---------------------------------------------------------------------------
#----- Install RUN
#---------------------------------------------------------------------------
win32 {
dll_opencv_rt.path = $${OUT_PWD}
dll_opencv_rt.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_core$${OPENCV_VER}.dll
dll_opencv_rt.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_imgproc$${OPENCV_VER}.dll
dll_opencv_rt.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_imgcodecs$${OPENCV_VER}.dll
dll_opencv_rt.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_videoio$${OPENCV_VER}.dll
dll_opencv_rt.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_highgui$${OPENCV_VER}.dll
dll_opencv_rt.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_tracking$${OPENCV_VER}.dll

INSTALLS += dll_opencv_rt
}
#---------------------------------------------------------------------------
#----- Install BIN
#---------------------------------------------------------------------------
win32 {
CONFIG(release, debug|release) {
    MINGW_VER = 73
    MINGW_Q = $${PLATFORM_VER}
    QT_DIR = C:/Qt

    OUT_DIR = $${PWD}/../_$${TARGET}_$${PLATFORM}
    QT_INSTALLED_PATH = $${QT_DIR}/$${QT_VERSION}/mingw$${MINGW_VER}_$${MINGW_Q}

    dll_bin.path = $${OUT_DIR}
    dll_bin.files += $${QT_INSTALLED_PATH}/bin/libgcc_s_dw2-1.dll
    dll_bin.files += $${QT_INSTALLED_PATH}/bin/libgcc_s_seh-1.dll
    dll_bin.files += $${QT_INSTALLED_PATH}/bin/libstdc++-6.dll
    dll_bin.files += $${QT_INSTALLED_PATH}/bin/libwinpthread-1.dll
    dll_bin.files += $${QT_INSTALLED_PATH}/bin/Qt5Charts.dll
    dll_bin.files += $${QT_INSTALLED_PATH}/bin/Qt5Core.dll
    dll_bin.files += $${QT_INSTALLED_PATH}/bin/Qt5Gui.dll
    dll_bin.files += $${QT_INSTALLED_PATH}/bin/Qt5Widgets.dll
    dll_bin.files += $${QT_INSTALLED_PATH}/bin/Qt5OpenGL.dll

    dll_platforms.path = $${OUT_DIR}/platforms
    dll_platforms.files += $${QT_INSTALLED_PATH}/plugins/platforms/qminimal.dll
    dll_platforms.files += $${QT_INSTALLED_PATH}/plugins/platforms/qoffscreen.dll
    dll_platforms.files += $${QT_INSTALLED_PATH}/plugins/platforms/qwindows.dll

    dll_styles.path = $${OUT_DIR}/styles
    dll_styles.files += $${QT_INSTALLED_PATH}/plugins/styles/qwindowsvistastyle.dll

    dll_imageformats.path = $${OUT_DIR}/imageformats
    dll_imageformats.files += $${QT_INSTALLED_PATH}/plugins/imageformats/qgif.dll
    dll_imageformats.files += $${QT_INSTALLED_PATH}/plugins/imageformats/qicns.dll
    dll_imageformats.files += $${QT_INSTALLED_PATH}/plugins/imageformats/qico.dll
    dll_imageformats.files += $${QT_INSTALLED_PATH}/plugins/imageformats/qjpeg.dll
    dll_imageformats.files += $${QT_INSTALLED_PATH}/plugins/imageformats/qsvg.dll
    dll_imageformats.files += $${QT_INSTALLED_PATH}/plugins/imageformats/qtga.dll
    dll_imageformats.files += $${QT_INSTALLED_PATH}/plugins/imageformats/qtiff.dll
    dll_imageformats.files += $${QT_INSTALLED_PATH}/plugins/imageformats/qwbmp.dll
    dll_imageformats.files += $${QT_INSTALLED_PATH}/plugins/imageformats/qwebp.dll

    dll_opencv.path = $${OUT_DIR}
    dll_opencv.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_core$${OPENCV_VER}.dll
    dll_opencv.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_imgproc$${OPENCV_VER}.dll
    dll_opencv.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_imgcodecs$${OPENCV_VER}.dll
    dll_opencv.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_videoio$${OPENCV_VER}.dll
    dll_opencv.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_highgui$${OPENCV_VER}.dll
    dll_opencv.files += $${OPENCV_SDK}/x$${PLATFORM_VER}/mingw/bin/libopencv_tracking$${OPENCV_VER}.dll

    target.path = $${OUT_DIR}

    INSTALLS += target dll_bin dll_platforms dll_styles dll_imageformats dll_opencv
}
}
unix {
CONFIG(release, debug|release) {
OUT_DIR = $${PWD}/../_$${TARGET}_$${PLATFORM}

target.path = $${OUT_DIR}

INSTALLS += target
}
}
