#include <QCoreApplication>

#include <iostream>
#include <fstream>
#include <chrono>
#include <unistd.h>

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
    const ChannelMap pre;
    Decoder decoder(fileName, pre);

    auto r = decoder.events();
    if (!r.empty())
    {
        Calibration calibration(pre, r);
    }
}

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);
    auto start = std::chrono::steady_clock::now();

    process("/home/egor/shares/tmp/proba_t2_thin_1");

//    process("/data/agp-c/polygon/kp_dynamic_1"); // file with bad block
    auto stop = std::chrono::steady_clock::now();
    std::cout << "Time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

    return 0;
//    return a.exec();
}
