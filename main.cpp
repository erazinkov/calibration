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
//    const auto pre = ChannelMap::mapNAP();
    const auto pre = ChannelMap::mapTMP();
//    auto start = std::chrono::steady_clock::now();
    Decoder decoder(fileName, pre);

    auto r = decoder.events();
//    auto stop = std::chrono::steady_clock::now();
//    std::cout << "Time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;
    if (!r.empty())
    {
        std::cout << "Events: " << r.size() << std::endl;
        Calibration calibration(pre, r);
    }
}

void a(int b) {

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

//    process("/home/egor/shares/tmp/test_period_10_sec_5_min_adcm_1"); //6866059
//    process("/home/egor/shares/tmp/test_period_1_sec_5_min_adcm_1"); //5540182

//    process("/home/egor/shares/tmp/phantom_3_20cm_1");
//    process("/home/egor/shares/tmp/jun19-18.32.43");
//    for (auto i{0}; i < 25; ++i) {
      // process("/home/egor/shares/tmp/jun19-16.59.46"); //field5
//    }
//      process("/home/egor/shares/tmp/jun19-17.52.17"); //field7
//      process("/home/egor/shares/tmp/jun19-18.32.43"); //field8
//    process("/home/egor/shares/tmp/jun16-16.27.36");
//      process("/home/egor/shares/tmp/phantom_3_20cm_1");
//process("/home/egor/shares/tmp/std_lenta_check_1");
    process("/home/egor/Downloads/c12_1");
//    process("/home/egor/shares/tmp/kp_static_1"); // file with bad block
    auto stop = std::chrono::steady_clock::now();
    std::cout << "Time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

//    uint64_t v1 = 3926860182;
//    uint32_t v2 = 3927035562;


//    uint32_t v1{0b1111'1111'1111'1111'1111'1111'1111'1111};
//    uint64_t v2{0b0000'0000'0000'0000'0000'0000'0000'0001'0000'0000'0000'0000'0000'0000'0000'0000};
//    uint32_t v3 = 52333;
//    std::cout << DBL_MAX << std::endl;

    return 0;
//    return a.exec();
}
