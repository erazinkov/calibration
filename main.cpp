#include <QCoreApplication>

#include <QTimer>
#include <QElapsedTimer>

#include <iostream>
#include <fstream>
#include <chrono>

#include "decoder.h"
#include "calibration.h"
#include "mapdata.h"
#include "geodata.h"

template<typename T> void getData(const std::string &fileName, std::vector<T> &data) {
    std::ifstream ifs;
    ifs.open(fileName, std::ios::in);
    if (!ifs.is_open())
    {
        std::cout << "Can't open file " << fileName << std::endl;
        return;
    }
    T d;
    while (ifs >> d)
    {
        data.push_back(d);
    }
}

void printTimePoint(const std::chrono::system_clock::time_point &timePoint);

void spinner();

void process(const std::string &mapFileName, const std::string &geoFileName);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTimer::singleShot(0, [] () {
        QElapsedTimer elapsedTimer;
        elapsedTimer.start();
        const QString mapFileName{"/home/egor/build-adcmmodifier-Desktop-Debug/adcm.dat.mod.map"};
        const QString geoFileName{"/home/egor/build-adcmmodifier-Desktop-Debug/test.txt"};
        process(mapFileName.toStdString(), geoFileName.toStdString());
        qInfo() << "Time elapsed, ms:" << elapsedTimer.elapsed();
        QCoreApplication::exit(0);
    });
    return a.exec();
}

void process(const std::string &mapFileName, const std::string &geoFileName)
{
    std::vector<MapData> mapData;
    std::vector<GeoData> geoData;

    getData(mapFileName, mapData);
    getData(geoFileName, geoData);

//    for (const auto& item : mapData)
//    {
//        printTimePoint(item.lastModified);
//    }

    for (const auto& item : geoData)
    {
        printTimePoint(item.period.first);
    }

//    const auto pre = ChannelMap::mapNAP();
//    Decoder decoder(mapFileName.toStdString(), pre);

//    auto r = decoder.events();
//    if (!r.empty())
//    {
//        std::cout << "Events: " << r.size() << std::endl;
//        Calibration calibration(pre, r);
//    }
}

void printTimePoint(const std::chrono::system_clock::time_point &timePoint)
{
    std::time_t tt{std::chrono::system_clock::to_time_t(timePoint)};
    std::cout << ctime(&tt);
}

void spinner()
{
    static int pos{0};
    const char cursor[4]{'|', '/', '-', '\\'};
    std::cout << "\r" << cursor[pos] << std::flush;
    pos = (pos + 1) % 4;
}
