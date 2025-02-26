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
    const auto pre = ChannelMap::mapNAP();
    Decoder decoder(fileName, pre);

    auto r = decoder.events();
    if (!r.empty())
    {
        std::cout << "Events: " << r.size() << std::endl;
        Calibration calibration(pre, r);
    }
}

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);
    auto start = std::chrono::steady_clock::now();

//    process("/home/egor/shares/tmp/std_mag_proba_1_1");

//    process("/home/egor/shares/tmp/test_period_10_sec_5min_1"); //8189809 / 7236445 = 1.13
//    process("/home/egor/shares/tmp/test_period_10_sec_5min_2"); //9373979 / 8261846 = 1.13
//    process("/home/egor/shares/tmp/test_period_1_sec_5min_1");  //7236445
//    process("/home/egor/shares/tmp/test_period_1_sec_5min_2");  //8261846

    process("/home/egor/shares/tmp/test_period_10_sec_5_min_adcm_1"); //6866059
    process("/home/egor/shares/tmp/test_period_1_sec_5_min_adcm_1"); //5540182

//    process("/home/egor/shares/tmp/phantom_3_20cm_1");

//    process("/data/agp-c/polygon/kp_dynamic_1"); // file with bad block
    auto stop = std::chrono::steady_clock::now();
    std::cout << "Time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

    return 0;
//    return a.exec();
}
