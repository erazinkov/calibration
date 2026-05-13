QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

QMAKE_CXXFLAGS_RELEASE += -O2

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        calibration2p.cpp \
        channel.cpp \
        channelmap.cpp \
        decoder.cpp \
        energypeak.cpp \
        eventsselector.cpp \
        histogrammanager.cpp \
        main.cpp \
        peakfinder.cpp \
        piecewiselinearfunction.cpp \
        polynomialfunction.cpp \
        timepeaksfinder.cpp


INCLUDEPATH += $$system(root-config --incdir)
LIBS += $$system(root-config --libs) -lMinuit -lSpectrum

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    adcm_df.h \
    calibration.h \
    calibration2p.h \
    channel.h \
    channelmap.h \
    decoder.h \
    energypeak.h \
    eventsselector.h \
    histogrammanager.h \
    peakfinder.h \
    piecewiselinearfunction.h \
    polynomialfunction.h \
    progressbar.h \
    spinner.h \
    timepeaksfinder.h \
    utils.h
