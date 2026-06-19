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

    auto r = decoder.events();
    auto c = decoder.counters();

    for (const auto &item : c.rawhits)
    {
        std::cout << item << " ";
    }
    std::cout << c.time << std::endl;

    if (!r.empty())
    {
        std::cout << "Events: " << r.size() << std::endl;
        Calibration calibration(path.stem().string(), pre, r);
    }
}

void a(int b) {

}

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);
    auto start = std::chrono::steady_clock::now();



    process("/home/egor/shares/tmp/coal/pulp_rot_c12_2cm_42cm_1");
//    process("/home/egor/shares/tmp/coal/pulp_rot_emptyness_1");
//    process("/home/egor/shares/tmp/coal/pulp_rot_c12_2cm_42cm_2");
//    process("/home/egor/shares/tmp/coal/pulp_rot_emptyness_pure_1");

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
