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
//        const QString mapFileName{"/home/egor/build-adcmmodifier-Desktop-Debug/adcm.dat.mod.map"};
        const QString mapFileName{"/home/egor/shares/tmp/tochka_1.mod.map"};
        const QString geoFileName{"/home/egor/shares/tmp/test.txt"};
        process(mapFileName.toStdString(), geoFileName.toStdString());
        qInfo() << "Time elapsed, ms:" << elapsedTimer.elapsed();
        QCoreApplication::exit(0);
    });
    return a.exec();
}

void strToNs()
{
    std::string str{"2024-05-01 00:00:00"};
    std::string strNs{"123456000"};
    std::tm tm = {};
    std::stringstream ss{str};
    ss >> std::get_time(&tm, "%Y-%m-%d  %H:%M:%S");
    std::chrono::system_clock::time_point tp{std::chrono::system_clock::from_time_t(std::mktime(&tm))};
    tp += std::chrono::nanoseconds(std::atoll(strNs.c_str()));
    long long int ns{tp.time_since_epoch().count()};
    std::cout << ns << std::endl;
}

void process(const std::string &mapFileName, const std::string &geoFileName)
{
//    strToNs();
    std::vector<MapData> mapData;
    std::vector<GeoData> geoData;

    getData(mapFileName, mapData);
    getData(geoFileName, geoData);

//    for (const auto& item : mapData)
//    {
//        printTimePoint(item.lastModified);
//    }

//    for (const auto& item : geoData)
//    {
//        printTimePoint(item.period.first);
//        printTimePoint(item.period.second);
//    }
    const std::string path{"/home/egor/shares/tmp/"};
    const auto pre = ChannelMap::mapNAP();
    Decoder decoder(pre);
    std::vector<dec_ev_t> events;

    const auto timeOffset{12'123'456'000};

    for (size_t i{0}; i < mapData.size(); ++i) {
        decoder.process(path + mapData.at(i).fileName, mapData.at(i).offset, std::pair<long long, long long>{ 1714510800123456000 - timeOffset, 1714512960123456000 + timeOffset});
        auto r = decoder.events();
        if (!r.empty())
        {
            std::cout << i << " Events: " << r.size() << std::endl;
            events.insert(events.cend(), r.cbegin(), r.cend());
        }
    }
    //ref: 202252 ref: 550199
    std::cout << "Total events: " << events.size() << std::endl; //59'047'117 ref: 59'224'785

//    for (const auto& geoItem : geoData) {
//        for (const auto& mapItem : mapData) {
//            if (geoItem.period.first < mapItem.lastModified && mapItem.lastModified < geoItem.period.second) {
//                decoder.process(path + mapItem.fileName, mapItem.offset, geoItem.getNanoSeconds());
//                auto r = decoder.events();
//                if (!r.empty())
//                {
//                    std::cout << "Events: " << r.size() << std::endl;
//                    Calibration calibration(pre, r);
//                }
//            }
//        }
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
