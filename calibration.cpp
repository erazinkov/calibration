#include "calibration.h"

#include <TCanvas.h>
#include <TError.h>
#include <TF1.h>
#include <TFile.h>
#include <TROOT.h>

#include "spinner.h"
#include "peakfinder.h"
#include "piecewiselinearfunction.h"
#include "polynomialfunction.h"

#include <sstream>

#include "utils.h"

Calibration::Calibration(const std::string &fileName, const ChannelMap &map, std::vector<dec_ev_t> &events)
    : fileName_(fileName), map_(map), events_(events)
{   
    idxsGamma_ = map.getIdxsByType(Channel::GAMMA);
    idxsAlpha_ = map.getIdxsByType(Channel::ALPHA);

    timePeaksFinder_ = std::make_unique<TimePeaksFinder>(map_);
    histogramManager_ = std::make_unique<HistogramManager>();

    process();
}

Calibration::~Calibration()
{

}

void Calibration::process()
{
//    processTimeStamp();
    processTime();
//    processAlphaCh();
    processGammaCh();
//    processGammaEnergyTime();
//    processGammaEnergy();
   processTimeWithEnergyCut();
   processGammaEnergyTime1();
}

std::vector<dec_ev_t> Calibration::selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha)
{
    std::vector<dec_ev_t> selectedEvents{};
    auto it{events_.begin()};

    while ( (it = std::find_if(it, events_.end(), [&idxGamma, &idxAlpha](dec_ev_t e){
                               return e.g.index == idxGamma && e.a.index == idxAlpha;
    })) != events_.end() ) {
        selectedEvents.push_back(*it);
        ++it;
    }
    return selectedEvents;
}

void Calibration::fillHistTime(const std::vector<dec_ev_t> &events, TH1 *h, double offset)
{
    for (const auto & item : events)
    {
        h->Fill(static_cast<double>(item.tdc) - offset);
    }
}

void Calibration::fillHistTimeWithEnergyCut(const std::vector<dec_ev_t> &events,
                                            TH1 *h,
                                            double offsetT,
                                            double minE,
                                            double maxE,
                                            bool exclude,
                                            TF1 f)
{
    for (const auto & item : events)
    {
        auto t{static_cast<double>(item.tdc)};
//        auto e{f.Eval(static_cast<double>(item.g.amp))};
        auto e{static_cast<double>(item.g.amp)};
        if (exclude)
        {
            if (e < minE || maxE < e)
            {
                h->Fill(t - offsetT);
            }
        }
        else
        {
            if (minE <= e && e <= maxE)
            {
                h->Fill(t - offsetT);
            }
        }
    }
}

void Calibration::fillHistEnergyTimeWithTimeCut(const std::vector<dec_ev_t> &events,
                                            TH2 *h,
                                            double minT,
                                            double maxT,
                                            bool exclude,
                                            TF1 f)
{
    for (const auto & item : events)
    {
        auto t{static_cast<double>(item.tdc)};
//        auto e{f.Eval(static_cast<double>(item.g.amp))};
        auto e{static_cast<double>(item.g.amp)};
        if (exclude)
        {
            if (t < minT || maxT < t)
            {
                h->Fill(e, t);
            }
        }
        else
        {
            if (minT <= t && t <= maxT)
            {
                h->Fill(e, t);
            }
        }
    }
}

void Calibration::fillHistEnergyTime(const std::vector<dec_ev_t> &events, TH2 *h, double offsetT, TF1 f)
{
    for (const auto & item : events)
    {
        auto t{static_cast<double>(item.tdc)};
//        auto e{f.Eval(static_cast<double>(item.g.amp))};
        auto e{static_cast<double>(item.g.amp)};
//        auto e{static_cast<double>(item.a.amp)};
        h->Fill(e, t - offsetT);
    }
}

void Calibration::fillHistChannel(const std::vector<dec_ev_t> &events, TH1 *h, double minT, double maxT, bool exclude)
{
    for (const auto & item : events)
    {
        auto t{static_cast<double>(item.tdc)};
        auto e{static_cast<double>(item.g.amp)};
        if (exclude)
        {
            if (t < minT || maxT < t)
            {
                h->Fill(e);
            }
        }
        else
        {
            if (minT <= t && t <= maxT)
            {
                h->Fill(e);
            }
        }
    }
}

void Calibration::fillHistChannelA(const std::vector<dec_ev_t> &events, TH1 *h)
{
    for (const auto & item : events)
    {
        auto e{static_cast<double>(item.a.amp)};
        h->Fill(e);
    }
}

void Calibration::fillHistEnergy(const std::vector<dec_ev_t> &events, TH1 *h, double minT, double maxT, bool exclude, TF1 f)
{
    for (const auto & item : events)
    {
        auto t{static_cast<double>(item.tdc)};
        auto e{static_cast<double>(item.g.amp)};
        if (exclude)
        {
            if (t < minT || maxT < t)
            {
                h->Fill(f.Eval(e));
            }
        }
        else
        {
            if (minT <= t && t <= maxT)
            {
                h->Fill(f.Eval(e));
            }
        }
    }
}

void Calibration::processTime()
{
    auto hists{histogramManager_->createHistograms("histTime", BINS_TIME, XLOW_TIME, XUP_TIME, idxsGamma_, idxsAlpha_)};

    auto start = std::chrono::steady_clock::now();

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            tasks.push_back([this, &hists, i, j](){
                auto sE{selectedEvents(static_cast<u_int8_t>(idxsGamma_.at(i)), static_cast<u_int8_t>(idxsAlpha_.at(j)))};
                fillHistTime(sE, hists.at(i).at(j).get(), 0.0);
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    auto stop = std::chrono::steady_clock::now();
    std::cout << "Time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

    const std::string outputFileNameTmp{"output_time_raw_" + fileName_ + ".root"};
    std::unique_ptr<TFile> fileTmp{TFile::Open((outputFileNameTmp).c_str(), "RECREATE")};
    if (fileTmp.get())
    {
        for (size_t i{0}; i < hists.size(); ++i)
        {
            for (size_t j{0}; j <  hists.at(i).size(); ++j)
            {
                hists.at(i).at(j).get()->Write(hists.at(i).at(j).get()->GetName(), TObject::kOverwrite);
            }
        }
    }

   timePeaksFinder_.get()->calculatePeaksPos(hists);



//   timePeaksFinder_.get()->readPeaksPosFromFile("time_peak_pos_c_12_new.txt");
//    timePeaksFinder_.get()->readPeaksPosFromFile("time_peak_pos_c12_bez_nijnej_zaschity.txt");
//    timePeaksFinder_.get()->writePeaksPosToFile("time_peak_pos_c12_bez_nijnej_zaschity.txt");

//   timePeaksFinder_.get()->readPeaksPosFromFile("time_peak_pos_c12_bez_nijnej_zaschity_w_energy_cut.txt");
//   timePeaksFinder_.get()->writePeaksPosToFile("time_peak_pos_c12_bez_nijnej_zaschity_w_energy_cut_check.txt");
//   timePeaksFinder_.get()->writePeaksPosToFile("time_peak_pos_sugar_emptiness_1.txt");
//   timePeaksFinder_.get()->readPeaksPosFromFile("time_peak_pos_sugar_emptiness_1.txt");

   auto hists_{histogramManager_->createHistograms("histTimeU", BINS_TIME, XLOW_TIME, XUP_TIME, idxsGamma_, idxsAlpha_)};

   for (size_t i{0}; i < hists_.size(); ++i)
   {
       for (size_t j{0}; j <  hists_.at(i).size(); ++j)
       {
           tasks.push_back([this, &hists_, i, j](){
               auto sE{selectedEvents(static_cast<u_int8_t>(idxsGamma_.at(i)), static_cast<u_int8_t>(idxsAlpha_.at(j)))};
               fillHistTime(sE, hists_.at(i).at(j).get(), timePeaksFinder_.get()->timePeaksPos().at(i).at(j));
           });
       }
   }
   func_async(tasks.begin(), tasks.end());
   tasks.clear();

   auto histsTimeAlpha{histogramManager_->createHistograms("histTimeAlpha", BINS_TIME, XLOW_TIME, XUP_TIME, idxsAlpha_)};
   for (size_t i{0}; i < hists_.size(); ++i)
   {
       for (size_t j{0}; j <  hists_.at(i).size(); ++j)
       {
           histsTimeAlpha.at(j).get()->Add(hists_.at(i).at(j).get());
       }
   }
   const std::string outputFileName{"output_time_" + fileName_ + ".root"};
   std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
   if (file.get())
   {
       for (auto &item : histsTimeAlpha)
       {
           item.get()->Write(item.get()->GetName(), TObject::kOverwrite);
       }
       for (size_t i{0}; i < hists_.size(); ++i)
       {
           for (size_t j{0}; j <  hists_.at(i).size(); ++j)
           {
               hists_.at(i).at(j).get()->Write(hists_.at(i).at(j).get()->GetName(), TObject::kOverwrite);
           }
       }
   }

    histogramManager_->printToPsFile("time", hists);
    histogramManager_->printToPsFile("timeAlpha", histsTimeAlpha);
//    _histogramManager->printToPsFile("time_1", hists.at(0).at(0));

}

void Calibration::processTimeWithEnergyCut()
{
    auto hists{histogramManager_->createHistograms("histTimeWithEnergyCut", BINS_TIME, XLOW_TIME, XUP_TIME, idxsGamma_, idxsAlpha_)};

    std::vector<TF1> fs;
    for (size_t i{0}; i < idxsGamma_.size(); ++i)
    {
        PiecewiseLinearFunction fObj(energyPeaks_.at(i));
        TF1 f("f", fObj, XLOW_CHANNEL, XUP_CHANNEL, 0);
        fs.push_back(f);
    }

    auto start = std::chrono::steady_clock::now();

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            tasks.push_back([this, &hists, i, j, &fs](){
                auto sE{selectedEvents(static_cast<u_int8_t>(idxsGamma_.at(i)), static_cast<u_int8_t>(idxsAlpha_.at(j)))};
                fillHistTimeWithEnergyCut(sE,
                                          hists.at(i).at(j).get(),
//                                          timePeaksFinder_.get()->timePeaksPos().at(i).at(j),
//                                          4438.0 - 240.0,
//                                          4438.0 + 240.0,
                                          0.0,
                                          800.0,
                                          900.0,
                                          false,
                                          fs.at(i));
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    auto stop = std::chrono::steady_clock::now();
    std::cout << "Time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

   timePeaksFinder_.get()->calculatePeaksPos(hists);
//   timePeaksFinder_.get()->writePeaksPosToFile("time_peak_pos_c12_bez_nijnej_zaschity_w_energy_cut.txt");

   auto histsTimeAlpha{histogramManager_->createHistograms("histTimeAlphaWithEnergyCut", BINS_TIME, XLOW_TIME, XUP_TIME, idxsAlpha_)};
   for (size_t i{0}; i < hists.size(); ++i)
   {
       for (size_t j{0}; j <  hists.at(i).size(); ++j)
       {
           histsTimeAlpha.at(j).get()->Add(hists.at(i).at(j).get());
       }
   }
   const std::string outputFileName{"output_time_" + fileName_ + "_w_energy_cut.root"};
   std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
   if (file.get())
   {
       for (auto &item : histsTimeAlpha)
       {
           item.get()->Write(item.get()->GetName(), TObject::kOverwrite);
       }
   }

//    _histogramManager->printToPsFile("time_withEnergyCut", hists);
//    _histogramManager->printToPsFile("timeAlpha_withEnergyCut", histsTimeAlpha);
//    _histogramManager->printToPsFile("time_1", hists.at(0).at(0));

}

void Calibration::processAlphaCh()
{
    auto hists{histogramManager_->createHistograms("histA", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, idxsGamma_, idxsAlpha_)};

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            tasks.push_back([this, i, j, &hists] () {
                auto sE{selectedEvents(static_cast<u_int8_t>(idxsGamma_.at(i)), static_cast<u_int8_t>(idxsAlpha_.at(j)))};
                fillHistChannelA(sE, hists.at(i).at(j).get());
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    auto histsAlpha{histogramManager_->createHistograms("histAlpha", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, idxsAlpha_)};
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j < hists.at(i).size(); ++j)
        {
            histsAlpha.at(j).get()->Add(hists.at(i).at(j).get());
        }
    }

    const std::string outputFileName{"output_energy_alpha_" + fileName_ + ".root"};

    std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
    if (file.get())
    {
        for (size_t i{0}; i < hists.size(); ++i)
        {
            for (size_t j{0}; j <  hists.at(i).size(); ++j)
            {
                hists.at(i).at(j).get()->Write(hists.at(i).at(j).get()->GetName(), TObject::kOverwrite);
            }
        }
        for (auto &item : histsAlpha)
        {
            item.get()->Write(item.get()->GetName(), TObject::kOverwrite);
        }
    }
}

void Calibration::processGammaCh()
{
    auto histsSg{histogramManager_->createHistograms("histSg", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, idxsGamma_, idxsAlpha_)};
    auto histsBg{histogramManager_->createHistograms("histBg", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, idxsGamma_, idxsAlpha_)};
    auto histsRc{histogramManager_->createHistograms("histRc", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, idxsGamma_, idxsAlpha_)};

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j)
        {
            tasks.push_back([this, i, j, &histsSg, &histsBg, &histsRc] () {
                auto sE{selectedEvents(static_cast<u_int8_t>(idxsGamma_.at(i)), static_cast<u_int8_t>(idxsAlpha_.at(j)))};
                auto tSgMin{timePeaksFinder_.get()->timePeaksPos().at(i).at(j) - 3.0};
                auto tSgMax{timePeaksFinder_.get()->timePeaksPos().at(i).at(j) + 3.0};
                fillHistChannel(sE, histsSg.at(i).at(j).get(), tSgMin, tSgMax, false);
                fillHistChannel(sE, histsRc.at(i).at(j).get(), tSgMin - 1.0, tSgMax + 1.0, true);
                auto tBgMin{timePeaksFinder_.get()->timePeaksPos().at(i).at(j) - 30.0};
                auto tBgMax{timePeaksFinder_.get()->timePeaksPos().at(i).at(j) - 20.0};
                fillHistChannel(sE, histsBg.at(i).at(j).get(), tBgMin, tBgMax, false);
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    auto histsSgGamma{histogramManager_->createHistograms("histSgGamma", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, idxsGamma_)};
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j < histsSg.at(i).size(); ++j)
        {
            histsSgGamma.at(i).get()->Add(histsSg.at(i).at(j).get());
            histsSgGamma.at(i).get()->Add(histsBg.at(i).at(j).get(), -6.0 / 10.0);
        }
    }
    auto histsRcGamma{histogramManager_->createHistograms("histRcGamma", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, idxsGamma_)};
    for (size_t i{0}; i < histsRc.size(); ++i)
    {
        for (size_t j{0}; j < histsRc.at(i).size(); ++j)
        {
            histsRcGamma.at(i).get()->Add(histsRc.at(i).at(j).get());
        }
    }
    PeakFinder peakFinder(map_);
    peakFinder.process(histsSgGamma, histsRcGamma);
    energyPeaks_ = peakFinder.energyPeaks();

}

void Calibration::processGammaEnergy()
{
    auto histsSg(histogramManager_->createHistograms("histSg", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, idxsGamma_, idxsAlpha_));
    auto histsBg(histogramManager_->createHistograms("histBg", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, idxsGamma_, idxsAlpha_));

    std::vector<TF1> fs;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        PiecewiseLinearFunction fObj(energyPeaks_.at(i));
        TF1 f("f", fObj, XLOW_CHANNEL, XUP_CHANNEL, 0);
        fs.push_back(f);
    }


    // save calibration
//    const std::string outputFileName1{"calibration_functions_" + fileName_ + ".root"};
//    std::unique_ptr<TFile> outputFile{TFile::Open((outputFileName1).c_str(), "RECREATE")};
//    if (outputFile.get())
//    {
//        for (size_t i{0}; i < fs.size(); ++i)
//        {
//            fs.at(i).Write(("f_" + std::to_string(i)).c_str(), TObject::kOverwrite);
//        }
//    }
    // load calibration
//    const std::string inputFileName{"calibration_functions_" + fileName_ + ".root"};
//    const std::string inputFileName{"calibration_functions_sugar_sulfur_1.root"};
//    std::unique_ptr<TFile> inputFile{TFile::Open((inputFileName).c_str(), "READ")};
//    if (inputFile.get())
//    {
//        for (size_t i{0}; i < fs.size(); ++i)
//        {
//            TObject *tObj{nullptr};
//            inputFile->GetObject(("f_" + std::to_string(i)).c_str(), tObj);
//            if (tObj)
//            {
//                fs[i] = *(static_cast<TF1 *>(tObj));
//            }
//            delete tObj;
//            tObj = nullptr;
//        }
//    }

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j)
        {
            tasks.push_back([this, i, j, &histsSg, &histsBg, &fs] () {
                auto sE{selectedEvents(static_cast<u_int8_t>(i), static_cast<u_int8_t>(j))};
                auto tSgMin{timePeaksFinder_.get()->timePeaksPos().at(i).at(j) - 3.0};
                auto tSgMax{timePeaksFinder_.get()->timePeaksPos().at(i).at(j) + 3.0};
                fillHistEnergy(sE, histsSg.at(i).at(j).get(), tSgMin, tSgMax, false, fs.at(i));
                auto tBgMin{timePeaksFinder_.get()->timePeaksPos().at(i).at(j) - 30.0};
                auto tBgMax{timePeaksFinder_.get()->timePeaksPos().at(i).at(j) - 20.0};
                fillHistEnergy(sE, histsBg.at(i).at(j).get(), tBgMin, tBgMax, false, fs.at(i));
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    auto histsSgGamma{histogramManager_->createHistograms("histSgGamma", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, idxsGamma_)};
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j < histsSg.at(i).size(); ++j)
        {
            histsSgGamma.at(i).get()->Add(histsSg.at(i).at(j).get());
            histsSgGamma.at(i).get()->Add(histsBg.at(i).at(j).get(), -6.0 / 10.0);
        }
    }

    std::shared_ptr<TH1D> hist{new TH1D("hist", "hist", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY)};
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        hist.get()->Add(histsSgGamma.at(i).get());
    }

    auto histsSgAlpha{histogramManager_->createHistograms("histSgAlpha", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, idxsAlpha_)};
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j < histsSg.at(i).size(); ++j)
        {
            histsSgAlpha.at(j).get()->Add(histsSg.at(i).at(j).get());
            histsSgAlpha.at(j).get()->Add(histsBg.at(i).at(j).get(), -6.0 / 10.0);
        }
    }
    histogramManager_->printToPsFile("eSgGamma", histsSgGamma);
    // _histogramManager->saveToRootFile("output", hist);
   const std::string outputFileName{"output_energy_" + fileName_ + ".root"};

   std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
   if (file.get())
   {
       for (auto &item : histsSgAlpha)
       {
           item.get()->Write(item.get()->GetName(), TObject::kOverwrite);
       }
       for (auto &item : histsSgGamma)
       {
           item.get()->Write(item.get()->GetName(), TObject::kOverwrite);
       }
       hist.get()->Write(hist.get()->GetName(), TObject::kOverwrite);
   }
}

void Calibration::processGammaEnergyTime1()
{
    auto hists(histogramManager_->createHistograms("histChannelTime1", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, BINS_TIME, XLOW_TIME, XUP_TIME, idxsGamma_, idxsAlpha_));

    std::vector<TF1> fs;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        PiecewiseLinearFunction fObj(energyPeaks_.at(i));
        TF1 f("f", fObj, XLOW_CHANNEL, XUP_CHANNEL, 0);
        fs.push_back(f);
    }

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            tasks.push_back([this, &hists, i, j, &fs](){
                auto sE{selectedEvents(static_cast<u_int8_t>(idxsGamma_.at(i)), static_cast<u_int8_t>(idxsAlpha_.at(j)))};
                fillHistEnergyTimeWithTimeCut(sE,
                                   hists.at(i).at(j).get(),
                                   timePeaksFinder_.get()->timePeaksPos().at(1).at(1) - 3.0,
                                   timePeaksFinder_.get()->timePeaksPos().at(1).at(1) + 3.0,
                                   false,
                                   fs.at(i));
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();


    const std::string outputFileName{"output_et_w_tc_" + fileName_ + "_raw.root"};
    std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
    if (file.get())
    {
        for (size_t i{0}; i < hists.size(); ++i)
        {
            for (size_t j{0}; j <  hists.at(i).size(); ++j)
            {
                hists.at(i).at(j).get()->Write(hists.at(i).at(j).get()->GetName(), TObject::kOverwrite);
            }
        }
    }
}

void Calibration::processGammaEnergyTime()
{
    auto hists(histogramManager_->createHistograms("histChannelTime", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, BINS_TIME, XLOW_TIME, XUP_TIME, idxsGamma_, idxsAlpha_));

    std::vector<TF1> fs;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        PiecewiseLinearFunction fObj(energyPeaks_.at(i));
        TF1 f("f", fObj, XLOW_CHANNEL, XUP_CHANNEL, 0);
        fs.push_back(f);
    }

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            tasks.push_back([this, &hists, i, j, &fs](){
                auto sE{selectedEvents(static_cast<u_int8_t>(idxsGamma_.at(i)), static_cast<u_int8_t>(idxsAlpha_.at(j)))};
                fillHistEnergyTime(sE,
                                   hists.at(i).at(j).get(),
                                   0.0,
                                   fs.at(i));
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();


    const std::string outputFileName{"output_et_" + fileName_ + "_raw.root"};
    std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
    if (file.get())
    {
        for (size_t i{0}; i < hists.size(); ++i)
        {
            for (size_t j{0}; j <  hists.at(i).size(); ++j)
            {
                hists.at(i).at(j).get()->Write(hists.at(i).at(j).get()->GetName(), TObject::kOverwrite);
            }
        }
    }
}

//void Calibration::processGammaEnergyTime()
//{
//    auto hists(_histogramManager->createHistograms("histEnergyTime", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, BINS_TIME, XLOW_TIME, XUP_TIME, _idxsGamma, _idxsAlpha));

//    std::vector<TF1> fs;
//    for (size_t i{0}; i < hists.size(); ++i)
//    {
//        PiecewiseLinearFunction fObj(_energyPeaks.at(i));
//        TF1 f("f", fObj, XLOW_CHANNEL, XUP_CHANNEL, 0);
//        fs.push_back(f);
//    }

//    // load calibration
////    const std::string inputFileName{"calibration_functions_" + fileName_ + ".root"};
////    const std::string inputFileName{"calibration_functions_sugar_sulfur_1.root"};
////    std::unique_ptr<TFile> inputFile{TFile::Open((inputFileName).c_str(), "READ")};
////    if (inputFile.get())
////    {
////        for (size_t i{0}; i < fs.size(); ++i)
////        {
////            TObject *tObj{nullptr};
////            inputFile->GetObject(("f_" + std::to_string(i)).c_str(), tObj);
////            if (tObj)
////            {
////                fs[i] = *(static_cast<TF1 *>(tObj));
////            }
////            delete tObj;
////            tObj = nullptr;
////        }
////    }

//    std::vector<std::function<void()>> tasks;
//    for (size_t i{0}; i < hists.size(); ++i)
//    {
//        for (size_t j{0}; j <  hists.at(i).size(); ++j)
//        {
//            tasks.push_back([this, &hists, i, j, &fs](){
//                auto sE{selectedEvents(static_cast<u_int8_t>(_idxsGamma.at(i)), static_cast<u_int8_t>(_idxsAlpha.at(j)))};
//                fillHistEnergyTime(sE,
//                                   hists.at(i).at(j).get(),
//                                   timePeaksFinder_.get()->timePeaksPos().at(i).at(j),
//                                   fs.at(i));
//            });
//        }
//    }
//    func_async(tasks.begin(), tasks.end());
//    tasks.clear();

//    auto histsEnergyTimeAlpha{_histogramManager->createHistograms("histEnergyTimeAlpha", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, BINS_TIME, XLOW_TIME, XUP_TIME, _idxsAlpha)};
//    for (size_t i{0}; i < hists.size(); ++i)
//    {
//        for (size_t j{0}; j <  hists.at(i).size(); ++j)
//        {
//            histsEnergyTimeAlpha.at(j).get()->Add(hists.at(i).at(j).get());
//        }
//    }
//    const std::string outputFileName{"output_et_" + fileName_ + ".root"};
////    const std::string outputFileName{"output_et_emptiness_1.root"};
////    const std::string outputFileName{"output_et_sio2_2kg_mask_1.root"};
//    std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
//    if (file->IsOpen())
//    {
//        for (auto &item : histsEnergyTimeAlpha)
//        {
//            item.get()->Write(item.get()->GetName(), TObject::kOverwrite);
//        }
//    }
//}
