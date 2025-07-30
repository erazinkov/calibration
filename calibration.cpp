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

Calibration::Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events) : _map(map), _events(events)
{
    _idxsGamma = map.getIdxsByType(Channel::GAMMA);
    _idxsAlpha = map.getIdxsByType(Channel::ALPHA);

    _timePeaksFinder = std::make_unique<TimePeaksFinder>(_map);
    _histogramManager = std::make_unique<HistogramManager>();

    process();
}

Calibration::~Calibration()
{

}

void Calibration::process()
{
//    processTimeStamp();
    processTime();
    processGammaCh();
    processGammaEnergy();
}

std::vector<dec_ev_t> Calibration::selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha)
{
    std::vector<dec_ev_t> selectedEvents{};
    auto it{_events.begin()};

    while ( (it = std::find_if(it, _events.end(), [&idxGamma, &idxAlpha](dec_ev_t e){
                               return e.g.index == idxGamma && e.a.index == idxAlpha;
    })) != _events.end() ) {
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

void Calibration::fillHistChannel(const std::vector<dec_ev_t> &events, TH1 *h, double min, double max, bool exclude)
{
    for (const auto & item : events)
    {
        auto t{static_cast<double>(item.tdc)};
        auto e{static_cast<double>(item.g.amp)};
        if (exclude)
        {
            if (t < min || max < t)
            {
                h->Fill(e);
            }
        }
        else
        {
            if (min <= t && t <= max)
            {
                h->Fill(e);
            }
        }
    }
}

void Calibration::fillHistEnergy(const std::vector<dec_ev_t> &events, TH1 *h, double min, double max, bool exclude, TF1 f)
{
    for (const auto & item : events)
    {
        auto t{static_cast<double>(item.tdc)};
        auto e{static_cast<double>(item.g.amp)};
        if (exclude)
        {
            if (t < min || max < t)
            {
                h->Fill(f.Eval(e));
            }
        }
        else
        {
            if (min <= t && t <= max)
            {
                h->Fill(f.Eval(e));
            }
        }
    }
}

void Calibration::processTime()
{
    auto hists{_histogramManager->createHistograms("histTime", BINS_TIME, XLOW_TIME, XUP_TIME, _idxsGamma, _idxsAlpha)};

    auto start = std::chrono::steady_clock::now();

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            tasks.push_back([this, &hists, i, j](){
                auto sE{selectedEvents(static_cast<u_int8_t>(_idxsGamma.at(i)), static_cast<u_int8_t>(_idxsAlpha.at(j)))};
                fillHistTime(sE, hists.at(i).at(j).get(), 0.0);
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    auto stop = std::chrono::steady_clock::now();
    std::cout << "Time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

    _timePeaksFinder.get()->calculatePeaksPos(hists);

    _histogramManager->printToPsFile("time", hists);
    _histogramManager->printToPsFile("time_1", hists.at(0).at(0));

}

void Calibration::processGammaCh()
{
    auto histsSg{_histogramManager->createHistograms("histSg", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, _idxsGamma, _idxsAlpha)};
    auto histsBg{_histogramManager->createHistograms("histBg", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, _idxsGamma, _idxsAlpha)};
    auto histsRc{_histogramManager->createHistograms("histRc", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, _idxsGamma, _idxsAlpha)};

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j)
        {
            tasks.push_back([this, i, j, &histsSg, &histsBg, &histsRc] () {
                auto sE{selectedEvents(static_cast<u_int8_t>(_idxsGamma.at(i)), static_cast<u_int8_t>(_idxsAlpha.at(j)))};
                auto tSgMin{_timePeaksFinder.get()->timePeaksPos().at(i).at(j) - 3.0};
                auto tSgMax{_timePeaksFinder.get()->timePeaksPos().at(i).at(j) + 3.0};
                fillHistChannel(sE, histsSg.at(i).at(j).get(), tSgMin, tSgMax, false);
                fillHistChannel(sE, histsRc.at(i).at(j).get(), tSgMin - 1.0, tSgMax + 1.0, true);
                auto tBgMin{_timePeaksFinder.get()->timePeaksPos().at(i).at(j) - 30.0};
                auto tBgMax{_timePeaksFinder.get()->timePeaksPos().at(i).at(j) - 20.0};
                fillHistChannel(sE, histsBg.at(i).at(j).get(), tBgMin, tBgMax, false);
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    auto histsSgGamma{_histogramManager->createHistograms("histSgGamma", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, _idxsGamma)};
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j < histsSg.at(i).size(); ++j)
        {
            histsSgGamma.at(i).get()->Add(histsSg.at(i).at(j).get());
            histsSgGamma.at(i).get()->Add(histsBg.at(i).at(j).get(), -6.0 / 10.0);
        }
    }
    auto histsRcGamma{_histogramManager->createHistograms("histRcGamma", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, _idxsGamma)};
    for (size_t i{0}; i < histsRc.size(); ++i)
    {
        for (size_t j{0}; j < histsRc.at(i).size(); ++j)
        {
            histsRcGamma.at(i).get()->Add(histsRc.at(i).at(j).get());
        }
    }
    PeakFinder peakFinder(_map);
    peakFinder.process(histsSgGamma, histsRcGamma);
    _energyPeaks = peakFinder.energyPeaks();

}

void Calibration::processGammaEnergy()
{
    auto histsSg(_histogramManager->createHistograms("histSg", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, _idxsGamma, _idxsAlpha));
    auto histsBg(_histogramManager->createHistograms("histBg", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, _idxsGamma, _idxsAlpha));

    std::vector<TF1> fs;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        PiecewiseLinearFunction fObj(_energyPeaks.at(i));
        TF1 f("f", fObj, XLOW_CHANNEL, XUP_CHANNEL, 0);
        fs.push_back(f);
    }

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j)
        {
            tasks.push_back([this, i, j, &histsSg, &histsBg, &fs] () {
                auto sE{selectedEvents(static_cast<u_int8_t>(i), static_cast<u_int8_t>(j))};
                auto tSgMin{_timePeaksFinder.get()->timePeaksPos().at(i).at(j) - 3.0};
                auto tSgMax{_timePeaksFinder.get()->timePeaksPos().at(i).at(j) + 3.0};
                fillHistEnergy(sE, histsSg.at(i).at(j).get(), tSgMin, tSgMax, false, fs.at(i));
                auto tBgMin{_timePeaksFinder.get()->timePeaksPos().at(i).at(j) - 30.0};
                auto tBgMax{_timePeaksFinder.get()->timePeaksPos().at(i).at(j) - 20.0};
                fillHistEnergy(sE, histsBg.at(i).at(j).get(), tBgMin, tBgMax, false, fs.at(i));
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    auto histsSgGamma{_histogramManager->createHistograms("histSgGamma", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, _idxsGamma)};
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
    _histogramManager->saveToRootFile("output", hist);
//    const std::string outputFileName{"output.root"};

//    std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
//    if (file->IsOpen())
//    {
//        hist.get()->Write(hist.get()->GetName(), TObject::kOverwrite);
//    }
}
