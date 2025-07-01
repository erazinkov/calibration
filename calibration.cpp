#include "calibration.h"

#include <TCanvas.h>
#include <TError.h>

#include <TF1.h>
#include <TFile.h>

#include "spinner.h"

Calibration::Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events) : _map(map), _events(events)
{
    _nGamma = map.numberOfChannelsGamma();
    _nAlpha = map.numberOfChannelsAlpha();

    _timePeaksPos.resize(_nGamma);
    for (auto & item : _timePeaksPos)
    {
        item.resize(_nAlpha, 0.0);
    }

    process();
}

void Calibration::process()
{
//    processTimeStamp();
    processTime();
    processGammaCh();
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
        if (exclude)
        {
            if (t < min || max < t)
            {
                h->Fill(static_cast<double>(item.g.amp));
            }
        }
        else
        {
            if (min <= t && t <= max)
            {
                h->Fill(static_cast<double>(item.g.amp));
            }
        }
    }
}

void Calibration::calculateTimePeaksPos(std::vector<std::vector<TH1 *> > &hists)
{
    gErrorIgnoreLevel = 3'000;
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists[ig].size(); ++ia)
        {
            _timePeaksPos[ig][ia] = calculateTimePeakPos(hists[ig][ia]);
        }
    }
    gErrorIgnoreLevel = 0;
}

double Calibration::calculateTimePeakPos(TH1 *hist) const
{
    auto timePeakPos{0.0};
    auto binMax{hist->GetMaximumBin()};
    auto xMax{hist->GetBinCenter(hist->GetBin(binMax))};
    auto rcAmp{hist->GetBinContent(hist->GetXaxis()->FindBin(xMax - 25.0))};
    auto peakAmp{hist->GetBinContent(binMax) - rcAmp};
    TF1 *f{new TF1("f", _timePeakFitFunctionObject, xMax - 25.0, xMax + 25.0, 5)};
    f->SetParameters(peakAmp, xMax, 5.0, rcAmp, 0.0);
    hist->Fit(f, "RQ");
    timePeakPos = f->GetParameter(1);
    delete f;
    f = nullptr;
    return timePeakPos;
}

void Calibration::drawHistsToFile(const std::string &psName, const std::vector<std::vector<TH1 *> > &hists) const
{
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c->Print((psName + '[').c_str());
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        auto cd{static_cast<int>(std::ceil(std::sqrt(hists[ig].size())))};
        c->Divide(cd, cd);
        for (size_t ia{0}; ia <  hists[ig].size(); ++ia)
        {
            c->cd(static_cast<int>(ia) + 1);
            hists[ig][ia]->Draw();
            auto listOfFunctions{hists[ig][ia]->GetListOfFunctions()};
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

void Calibration::deleteHists(std::vector<std::vector<TH1 *> > &hists)
{
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            delete hists[i][j];
            hists[i][j] = nullptr;
        }
    }
}

void Calibration::processTimeStamp()
{

}

void Calibration::processTime()
{
    std::vector<std::vector<TH1 *>> hists(_nGamma);
    for (size_t i{0}; i < hists.size(); ++i)
    {
        hists.at(i).resize(_nAlpha);
    }
    prepareHists("histTime", 400, -100, 100, hists);

    std::vector<std::future<void>> futures;
    for (size_t i{0}; i < hists.size(); ++i)
    {
        for (size_t j{0}; j <  hists.at(i).size(); ++j)
        {
            futures.emplace_back(std::async(std::launch::async, [this] (u_int8_t g, u_int8_t a, TH1 *h) {
                auto sE{selectedEvents(g, a)};
                fillHistTime(sE, h, 0.0);
            }, i, j, hists.at(i).at(j)));
        }
    }

    for (size_t i{0}; i < futures.size(); ++i)
    {
        futures[i].get();
    }

    futures.clear();

    calculateTimePeaksPos(hists);

    const std::string psName{"time.ps"};
    drawHistsToFile(psName, hists);

    clearHists(hists);
    deleteHists(hists);
}

void Calibration::processGammaCh()
{
    std::vector<std::vector<TH1 *>> histsSg(_nGamma);
    std::vector<std::vector<TH1 *>> histsBg(_nGamma);
    std::vector<std::vector<TH1 *>> histsD(_nGamma);
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        histsSg.at(i).resize(_nAlpha);
        histsBg.at(i).resize(_nAlpha);
        histsD.at(i).resize(_nAlpha);
    }
    prepareHists("histSg", 640, 0, 4e3, histsSg);
    prepareHists("histBg", 640, 0, 4e3, histsBg);
    prepareHists("histD", 640, 0, 4e3, histsD);

    std::vector<std::future<void>> futures;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j)
        {
            futures.emplace_back(std::async(std::launch::async, [this] (u_int8_t g, u_int8_t a, TH1 *hSg, TH1 *hBg) {
                auto sE{selectedEvents(g, a)};
                auto tSgMin{_timePeaksPos.at(g).at(a) - 3.0};
                auto tSgMax{_timePeaksPos.at(g).at(a) + 3.0};
                fillHistChannel(sE, hSg, tSgMin, tSgMax, false);
                auto tBgMin{_timePeaksPos.at(g).at(a) - 30.0};
                auto tBgMax{_timePeaksPos.at(g).at(a) - 20.0};
                fillHistChannel(sE, hBg, tBgMin, tBgMax, false);
            }, i, j, histsSg.at(i).at(j), histsBg.at(i).at(j)));

        }
    }

    for (size_t i{0}; i < futures.size(); ++i)
    {
        futures[i].get();
    }

    futures.clear();


    for (size_t i{0}; i < histsD.size(); ++i)
    {
        for (size_t j{0}; j < histsD.at(i).size(); ++j)
        {
            histsD.at(i).at(j)->Add(histsSg.at(i).at(j));
            histsD.at(i).at(j)->Add(histsBg.at(i).at(j), -6.0 / 10.0);
        }
    }


    const std::string psName{"gamma_ch.ps"};

    drawHistsToFile(psName, histsD);

    clearHists(histsSg);
    deleteHists(histsSg);

    clearHists(histsBg);
    deleteHists(histsBg);
}
