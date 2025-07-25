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


Calibration::Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events) : _map(map), _events(events)
{
    _idxsGamma = map.getIdxsByType(Channel::GAMMA);
    _idxsAlpha = map.getIdxsByType(Channel::ALPHA);

    _nGamma = map.numberOfChannels(Channel::GAMMA);
    _nAlpha = map.numberOfChannels(Channel::ALPHA);

    _timePeaksPos.resize(_nGamma);
    for (auto & item : _timePeaksPos)
    {
        item.resize(_nAlpha, 0.0);
    }

    _par.resize(_nGamma);


    _timePeaksFinder = std::make_unique<TimePeaksFinder>(_map);

    process();
}

Calibration::~Calibration()
{

}

void Calibration::process()
{
//    processTimeStamp();
    processTime();
//    processGammaCh();
//    processGammaEnergy();
}

std::vector<dec_ev_t> Calibration::selectedEvents(uint8_t ig, u_int8_t ia)
{
    std::vector<dec_ev_t> selectedEvents{};
    auto it{_events.begin()};

    while ( (it = std::find_if(it, _events.end(), [&ig, &ia](dec_ev_t e){
                               return e.g.index == ig && e.a.index == ia;
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

void Calibration::drawHistsToFile(const std::string &psName, const std::vector<std::vector<std::shared_ptr<TH1>> > &hists) const
{
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c->Print((psName + '[').c_str());
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        auto cd{static_cast<int>(std::ceil(std::sqrt(hists.at(ig).size())))};
        c->Divide(cd, cd);
        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
        {
            c->cd(static_cast<int>(ia) + 1);
            hists.at(ig).at(ia)->Draw();
            auto listOfFunctions{hists.at(ig).at(ia)->GetListOfFunctions()};
            for (auto *item : *listOfFunctions)
            {
                item->Draw("SAME");
            }
        }
        c->Print(psName.c_str());
        c->Clear();
    }
    c->Print((psName + ']').c_str());
    gErrorIgnoreLevel = 0;
}

void Calibration::prepareHists(const std::string &histName, int nBinsX, double xLow, double xUp, std::vector<std::vector<std::shared_ptr<TH1>> > &hists)
{
    std::stringstream ss;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            hists.at(i).at(j).reset();
            ss.clear();ss.str("");
            ss << histName << "_" << i << "_" << j;
            auto h = std::make_shared<TH1D>(ss.str().c_str(), ss.str().c_str(), nBinsX, xLow, xUp);
            h->Sumw2();
            hists.at(i).at(j) = h;
        }
    }
}

void Calibration::prepareHists(const std::string &histName, int nBinsX, double xLow, double xUp, std::vector<std::shared_ptr<TH1>> &hists)
{
    std::stringstream ss;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        hists.at(i).reset();
        ss.clear();ss.str("");
        ss << histName << "_" << i;
        auto h = std::make_shared<TH1D>(ss.str().c_str(), ss.str().c_str(), nBinsX, xLow, xUp);
        h->Sumw2();
        hists.at(i) = h;
    }
}

void Calibration::processTimeStamp()
{

}

void Calibration::processTime()
{
    std::vector<std::vector<std::shared_ptr<TH1>>> hists(_idxsGamma.size());

    for (auto &item : hists)
    {
        item.resize(_idxsAlpha.size(), nullptr);
    }

    prepareHists("histTime", BINS_TIME, XLOW_TIME, XUP_TIME, hists);

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


//    _timePeaksFinder.get()->calculatePeaksPos(hists);

    const std::string psName{"time.ps"};
    drawHistsToFile(psName, hists);

}

void Calibration::processGammaCh()
{
    std::vector<std::vector<std::shared_ptr<TH1>>> histsSg(_nGamma);
    std::vector<std::vector<std::shared_ptr<TH1>>> histsBg(_nGamma);
    std::vector<std::vector<std::shared_ptr<TH1>>> histsRc(_nGamma);
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        histsSg.at(i).resize(_nAlpha, nullptr);
        histsBg.at(i).resize(_nAlpha, nullptr);
        histsRc.at(i).resize(_nAlpha, nullptr);
    }
    prepareHists("histSg", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, histsSg);
    prepareHists("histBg", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, histsBg);
    prepareHists("histRc", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, histsRc);

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j)
        {
            tasks.push_back([this, i, j, &histsSg, &histsBg, &histsRc] () {
                auto sE{selectedEvents(static_cast<u_int8_t>(i), static_cast<u_int8_t>(j))};
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

    std::vector<std::shared_ptr<TH1>> histsSgGamma(_nGamma, nullptr);
    prepareHists("histSgGamma", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, histsSgGamma);

    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j < histsSg.at(i).size(); ++j)
        {
            histsSgGamma.at(i).get()->Add(histsSg.at(i).at(j).get());
            histsSgGamma.at(i).get()->Add(histsBg.at(i).at(j).get(), -6.0 / 10.0);
        }
    }


    std::vector<std::shared_ptr<TH1>> histsRcGamma(_nGamma, nullptr);
    prepareHists("histRcGamma", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, histsRcGamma);

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

//    const std::string psName{"gamma_ch.ps"};
//    drawHistsToFile(psName, histsD);
}

void Calibration::processGammaEnergy()
{
    std::vector<std::vector<std::shared_ptr<TH1>>> histsSg(_nGamma);
    std::vector<std::vector<std::shared_ptr<TH1>>> histsBg(_nGamma);
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        histsSg.at(i).resize(_nAlpha, nullptr);
        histsBg.at(i).resize(_nAlpha, nullptr);
    }
    prepareHists("histSg", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, histsSg);
    prepareHists("histBg", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, histsBg);

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

    std::vector<std::shared_ptr<TH1>> histsSgGamma(_nGamma, nullptr);
    prepareHists("histSgGamma", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, histsSgGamma);

    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j < histsSg.at(i).size(); ++j)
        {
            histsSgGamma.at(i).get()->Add(histsSg.at(i).at(j).get());
            histsSgGamma.at(i).get()->Add(histsBg.at(i).at(j).get(), -6.0 / 10.0);
        }
    }

    std::unique_ptr<TH1D> hist{new TH1D("hist", "hist", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY)};

    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        hist.get()->Add(histsSgGamma.at(i).get());
    }

    const std::string outputFileName{"field5_r.root"};

    std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
    if (file->IsOpen())
    {
        hist.get()->Write(hist.get()->GetName(), TObject::kOverwrite);
    }
}
