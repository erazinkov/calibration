#include "calibration.h"

#include <TCanvas.h>
#include <TError.h>

#include <TF1.h>
#include <TFile.h>

#include "spinner.h"

#include "peakfinder.h"

#include <functional>

#include <TROOT.h>

Calibration::Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events) : _map(map), _events(events)
{
    _nGamma = map.numberOfChannelsGamma();
    _nAlpha = map.numberOfChannelsAlpha();

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

void Calibration::drawHistsToFile(const std::string &psName, const std::vector<std::vector<TH1 *> > &hists) const
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


void Calibration::prepareHists(const std::string &histName, int nBinsX, double xLow, double xUp, std::vector<std::vector<TH1 *> > &hists)
{
    std::stringstream ss;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            ss.clear();ss.str("");
            ss << histName << "_" << i << "_" << j;
            TH1 *h{new TH1D(ss.str().c_str(), ss.str().c_str(), nBinsX, xLow, xUp)};
            h->Sumw2();
            hists.at(i).at(j) = h;
        }
    }
}

void Calibration::prepareHists(const std::string &histName, int nBinsX, double xLow, double xUp, std::vector<TH1 *> &hists)
{
    std::stringstream ss;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        ss.clear();ss.str("");
        ss << histName << "_" << i;
        TH1 *h{new TH1D(ss.str().c_str(), ss.str().c_str(), nBinsX, xLow, xUp)};
        h->Sumw2();
        hists.at(i) = h;
    }
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

void Calibration::clearHists(std::vector<std::vector<TH1 *> > &hists)
{
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            hists.at(i).at(j)->Reset();
        }
    }
}

void Calibration::clearHists(std::vector<TH1 *> &hists)
{
    for (size_t i{0}; i < hists.size(); ++i)
    {
        hists.at(i)->Reset();
    }
}

void Calibration::deleteHists(std::vector<std::vector<TH1 *> > &hists)
{
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            delete hists.at(i).at(j);
            hists.at(i).at(j) = nullptr;
        }
    }
}

void Calibration::deleteHists(std::vector<TH1 *> &hists)
{
    for (size_t i{0}; i < hists.size(); ++i)
    {
        delete hists.at(i);
        hists.at(i) = nullptr;
    }
}

void Calibration::processTimeStamp()
{

}

void Calibration::processTime()
{
    std::vector<std::vector<std::shared_ptr<TH1>>> _hists(_nGamma);

    for (size_t i{0}; i < _hists.size(); ++i)
    {
        _hists.at(i).resize(_nAlpha, nullptr);
    }

    prepareHists("histTime", BINS_TIME, XLOW_TIME, XUP_TIME, _hists);

    auto start = std::chrono::steady_clock::now();

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < _hists.size(); ++i)
    {
        for (size_t j{0}; j <  _hists.at(i).size(); ++j)
        {
            tasks.push_back([this, &_hists, i, j](){
                auto sE{selectedEvents(static_cast<u_int8_t>(i), static_cast<u_int8_t>(j))};
                fillHistTime(sE, _hists.at(i).at(j).get(), 0.0);
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    auto stop = std::chrono::steady_clock::now();
    std::cout << "Time elapsed, ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;


    _timePeaksFinder.get()->calculatePeaksPos(_hists);
//    calculateTimePeaksPos(hists);

    const std::string psName{"time.ps"};
    drawHistsToFile(psName, _hists);
//    clearHists(hists);
//    deleteHists(hists);
}

void Calibration::processGammaCh()
{
    std::vector<std::vector<TH1 *>> histsSg(_nGamma);
    std::vector<std::vector<TH1 *>> histsBg(_nGamma);
    std::vector<std::vector<TH1 *>> histsRc(_nGamma);
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
                auto tSgMin{_timePeaksPos.at(i).at(j) - 3.0};
                auto tSgMax{_timePeaksPos.at(i).at(j) + 3.0};
                fillHistChannel(sE, histsSg.at(i).at(j), tSgMin, tSgMax, false);
                fillHistChannel(sE, histsRc.at(i).at(j), tSgMin - 1.0, tSgMax + 1.0, true);
                auto tBgMin{_timePeaksPos.at(i).at(j) - 30.0};
                auto tBgMax{_timePeaksPos.at(i).at(j) - 20.0};
                fillHistChannel(sE, histsBg.at(i).at(j), tBgMin, tBgMax, false);
            });
        }
    }

    func_async(tasks.begin(), tasks.end());
    tasks.clear();

    std::vector<TH1 *> histsSgGamma(_nGamma, nullptr);
    prepareHists("histSgGamma", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, histsSgGamma);

    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j < histsSg.at(i).size(); ++j)
        {
            histsSgGamma.at(i)->Add(histsSg.at(i).at(j));
            histsSgGamma.at(i)->Add(histsBg.at(i).at(j), -6.0 / 10.0);
        }
    }


    std::vector<TH1 *> histsRcGamma(_nGamma, nullptr);
    prepareHists("histRcGamma", BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL, histsRcGamma);

    for (size_t i{0}; i < histsRc.size(); ++i)
    {
        for (size_t j{0}; j < histsRc.at(i).size(); ++j)
        {
            histsRcGamma.at(i)->Add(histsRc.at(i).at(j));
        }
    }

    clearHists(histsSg);
    clearHists(histsBg);
    clearHists(histsRc);
    deleteHists(histsSg);
    deleteHists(histsBg);
    deleteHists(histsRc);

//    PeakFinder peakFinder;
//    peakFinder.process(histsSgGamma, histsRcGamma);

//    auto p{peakFinder.getPar()};
//    for (size_t i{0}; i < p.size(); ++i)
//    {
//        for (size_t j{0}; j < p.at(i).size(); ++j)
//        {
//            _par.at(i).push_back(p.at(i).at(j));
//        }
//    }

    clearHists(histsSgGamma);
    clearHists(histsRcGamma);
    deleteHists(histsSgGamma);
    deleteHists(histsRcGamma);

//    const std::string psName{"gamma_ch.ps"};
//    drawHistsToFile(psName, histsD);



}

void Calibration::processGammaEnergy()
{
    std::vector<std::vector<TH1 *>> histsSg(_nGamma);
    std::vector<std::vector<TH1 *>> histsBg(_nGamma);
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        histsSg.at(i).resize(_nAlpha, nullptr);
        histsBg.at(i).resize(_nAlpha, nullptr);
    }
    prepareHists("histSg", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, histsSg);
    prepareHists("histBg", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, histsBg);

    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j)
        {
            tasks.push_back([this, i, j, &histsSg, &histsBg] () {
                auto sE{selectedEvents(static_cast<u_int8_t>(i), static_cast<u_int8_t>(j))};
                auto tSgMin{_timePeaksPos.at(i).at(j) - 3.0};
                auto tSgMax{_timePeaksPos.at(i).at(j) + 3.0};
                TF1 f("f", "pol3", XLOW_ENERGY, XUP_ENERGY);
                fillHistEnergy(sE, histsSg.at(i).at(j), tSgMin, tSgMax, false, f);
                fillHistChannel(sE, histsSg.at(i).at(j), tSgMin, tSgMax, false);
                auto tBgMin{_timePeaksPos.at(i).at(j) - 30.0};
                auto tBgMax{_timePeaksPos.at(i).at(j) - 20.0};
                fillHistEnergy(sE, histsBg.at(i).at(j), tBgMin, tBgMax, false, f);
            });
        }
    }

    func_async(tasks.begin(), tasks.end());

    tasks.clear();

    std::vector<TH1 *> histsSgGamma(_nGamma, nullptr);
    prepareHists("histSgGamma", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY, histsSgGamma);

    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j < histsSg.at(i).size(); ++j)
        {
            histsSgGamma.at(i)->Add(histsSg.at(i).at(j));
            histsSgGamma.at(i)->Add(histsBg.at(i).at(j), -6.0 / 10.0);
        }
    }

    clearHists(histsSg);
    clearHists(histsBg);
    deleteHists(histsSg);
    deleteHists(histsBg);

    std::unique_ptr<TH1D> hist{new TH1D("hist", "hist", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY)};

    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        hist.get()->Add(histsSgGamma.at(i));
    }

    clearHists(histsSgGamma);
    deleteHists(histsSgGamma);

    const std::string outputFileName{"field5_r.root"};

    std::unique_ptr<TFile> file{TFile::Open((outputFileName).c_str(), "RECREATE")};
    if (file->IsOpen())
    {
        hist.get()->Write(hist.get()->GetName(), TObject::kOverwrite);
    }
}
