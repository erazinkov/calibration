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

    auto r = decoder.events_3_p();
    auto c = decoder.counters();

    for (const auto &item : c.rawhits)
    {
        std::cout << item << " ";
    }
    std::cout << c.time << std::endl;

    if (!r.empty())
    {
        std::cout << "Events: " << r.size() << std::endl;
//        Calibration calibration(path.stem().string(), pre, r);
    }
}

void a(int b) {

}

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);
    auto start = std::chrono::steady_clock::now();
    process("/home/egor/shares/tmp/data_3_p/data_3_p_1");
    auto stop = std::chrono::steady_clock::now();
    std::cout << "Total time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

    return 0;
//    return a.exec();
}
