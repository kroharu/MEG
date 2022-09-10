QT       += core gui charts widgets serialport printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cplot.cpp \
    main.cpp \
    mainwindow.cpp \
    measurement.cpp \
    qcustomplot.cpp \
    selectchart.cpp \
    setting.cpp \
    source/Bessel.cpp \
    source/Biquad.cpp \
    source/Butterworth.cpp \
    source/Cascade.cpp \
    source/ChebyshevI.cpp \
    source/ChebyshevII.cpp \
    source/Custom.cpp \
    source/Design.cpp \
    source/Documentation.cpp \
    source/Elliptic.cpp \
    source/Filter.cpp \
    source/Legendre.cpp \
    source/Param.cpp \
    source/PoleFilter.cpp \
    source/RBJ.cpp \
    source/RootFinder.cpp \
    source/State.cpp \
    workwithfiles.cpp

HEADERS += \
    Dsp.h \
    DspFilters/Bessel.h \
    DspFilters/Biquad.h \
    DspFilters/Butterworth.h \
    DspFilters/Cascade.h \
    DspFilters/ChebyshevI.h \
    DspFilters/ChebyshevII.h \
    DspFilters/Common.h \
    DspFilters/Custom.h \
    DspFilters/Design.h \
    DspFilters/Dsp.h \
    DspFilters/Elliptic.h \
    DspFilters/Filter.h \
    DspFilters/Layout.h \
    DspFilters/Legendre.h \
    DspFilters/MathSupplement.h \
    DspFilters/Params.h \
    DspFilters/PoleFilter.h \
    DspFilters/RBJ.h \
    DspFilters/RootFinder.h \
    DspFilters/SmoothedFilter.h \
    DspFilters/State.h \
    DspFilters/Types.h \
    DspFilters/Utilities.h \
    cplot.h \
    ftd2xx.h \
    NIDAQmx.h \
    mainwindow.h \
    measurement.h \
    qcustomplot.h \
    selectchart.h \
    setting.h \
    workwithfiles.h

FORMS += \
    cplot.ui \
    mainwindow.ui \
    selectchart.ui \
    setting.ui

LIBS += \
    ~/Desktop/MEG/Linux/Quspin/libftd2xx.so.1.4.27 \
    ~/Desktop/MEG/Linux/Quspin/libnidaqmx.so.22.5.0 \
    ~/Desktop/MEG/Linux/Quspin/nisyscfg.lib


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
