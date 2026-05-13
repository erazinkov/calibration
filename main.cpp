#include <QCoreApplication>

#include <iostream>
#include <fstream>
#include <chrono>
#include <unistd.h>
#include <filesystem>

#include "decoder.h"
#include "calibration.h"

//void spinner()
//{
//    static int pos{0};
//    const char cursor[4]{'|', '/', '-', '\\'};
//    std::cout << "\r" << cursor[pos] << std::flush;
//    pos = (pos + 1) % 4;
//}

void process(const std::string fileName)
{

    auto pathWithFileName{fileName};
    std::filesystem::path path{pathWithFileName};

//    const auto pre = ChannelMap::mapNAP();
    const auto pre = ChannelMap::mapTMP();
    Decoder decoder(fileName, pre);
//    auto p = decoder.pulses();
//    for (auto it = p.begin(); it != p.end(); ++it) {
//        std::cout << it->first << ": " << it->second << "\n";
//    }
    auto r_2p = decoder.events_2p();
    auto r_3p = decoder.events_3p();
    qDebug() << r_2p.size() << r_3p.size();
//    auto c = decoder.counters();

//    for (const auto &item : c.rawhits)
//    {
//        std::cout << item << " ";
//    }
//    std::cout << c.time << std::endl;

//    if (!r.empty())
//    {
//        std::cout << "Events: " << r.size() << std::endl;
//        Calibration calibration(path.stem().string(), pre, r);
//    }
}

void a(int b) {

}

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);
    auto start = std::chrono::steady_clock::now();
//a: 87730
//aa: 3466
//aaa: 124
//aaaaag: 6
//aaaag: 437
//aaag: 16464
//aag: 474271
//ag: 12270520
//g: 256

//     process("/home/egor/shares/tmp/adcm-test/pulp_sugar8kg_47cm_multipl_2to7_1");
    process("/home/egor/shares/tmp/adcm-test/pulp_sugar8kg_47cm_multipl_2to7_reversed_1");
//    process("/home/egor/shares/tmp/adcm-test/pulp_sugar8kg_47cm_multipl_2to7_low_thr_reversed_1");
    auto stop = std::chrono::steady_clock::now();
    std::cout << "Total time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

//    uint64_t v1 = 3926860182;
//    uint32_t v2 = 3927035562;


//    uint32_t v1{0b1111'1111'1111'1111'1111'1111'1111'1111};
//    uint64_t v2{0b0000'0000'0000'0000'0000'0000'0000'0001'0000'0000'0000'0000'0000'0000'0000'0000};
//    uint32_t v3 = 52333;
//    std::cout << DBL_MAX << std::endl;

    return 0;
//    return a.exec();
}
