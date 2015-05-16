TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    framecalculation.cpp

LIBS += D:\Programy\OpenCV_2.3.1\mybuild\install\bin\libopencv_core231.dll
LIBS += D:\Programy\OpenCV_2.3.1\mybuild\install\bin\libopencv_contrib231.dll
LIBS += D:\Programy\OpenCV_2.3.1\mybuild\install\bin\libopencv_highgui231.dll
LIBS += D:\Programy\OpenCV_2.3.1\mybuild\install\bin\libopencv_imgproc231.dll
LIBS += D:\Programy\OpenCV_2.3.1\mybuild\install\bin\libopencv_calib3d231.dll
LIBS += D:\Programy\OpenCV_2.3.1\mybuild\install\bin\opencv_ffmpeg.dll

INCLUDEPATH += D:/Programy/OpenCV_2.3.1/build/install/include
DEPENDPATH += D:/Programy/OpenCV_2.3.1/build/install/include

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    framecalculation.h

