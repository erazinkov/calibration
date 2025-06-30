#include "calibration.h"

#include <TCanvas.h>
#include <TError.h>

#include <TF1.h>
#include <TFile.h>

Calibration::Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events) : map_(map), events_(events)
{
    nGamma_ = map.numberOfChannelsGamma();
    nAlpha_ = map.numberOfChannelsAlpha();

    timePeaksPos_.resize(nGamma_);
    for (auto & item : timePeaksPos_)
    {
        item.resize(nAlpha_, 0.0);
    }

    process();
}

void Calibration::process()
{
//    processTimeStamp();
    processTime();
    processGammaAmp();
}

std::vector<dec_ev_t> Calibration::selectedEvents(uint8_t ig, u_int8_t ia)
{
    std::vector<dec_ev_t> selectedEvents{};
    auto it{events_.begin()};

    while ( (it = std::find_if(it, events_.end(), [&ig, &ia](dec_ev_t e){
                               return e.g.index == ig && e.a.index == ia;
})) != events_.end() ) {
        selectedEvents.push_back(*it);
        ++it;
    }
    return selectedEvents;
}

double Calibration::valueTimeStamp(const dec_ev_t &event)
{
    return static_cast<double>(event.ts);
}

double Calibration::valueTime(const dec_ev_t &event)
{
    return static_cast<double>(event.tdc) - timePeaksPos_[event.g.index][event.a.index];
}

double Calibration::valueGammaAmp(const dec_ev_t &event)
{
    return static_cast<double>(event.g.amp);
}

void Calibration::calculateTimePeaksPos(const std::vector<std::vector<TH1 *> > &hists)
{
    gErrorIgnoreLevel = 3'000;
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists[ig].size(); ++ia)
        {
            timePeaksPos_[ig][ia] = calculateTimePeakPos(hists[ig][ia]);
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
    TF1 *f{new TF1("f", timePeakFitFunctionObject_, xMax - 25.0, xMax + 25.0, 5)};
    f->SetParameters(peakAmp, xMax, 5.0, rcAmp, 0.0);
    hist->Fit(f, "RQ");
    timePeakPos = f->GetParameter(1);
    delete f;
    f = nullptr;
    return timePeakPos;
}

void Calibration::fillHist(const std::vector<dec_ev_t> &events, TH1 *h, std::function<double(const dec_ev_t &)> f)
{
    for (const auto & item : events)
    {
        auto v{f(item)};
        h->Fill(v);
    }
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
    for (size_t ig{0}; ig < nGamma_; ++ig)
    {
        for (size_t ia{0}; ia <  nAlpha_; ++ia)
        {
            ss.clear();ss.str("");
            ss << histName << "_" << ig << "_" << ia;
            TH1 *h{new TH1D(ss.str().c_str(), ss.str().c_str(), nBinsX, xLow, xUp)};
            hists[ig].push_back(h);
        }
    }
}

void Calibration::clearHists(std::vector<std::vector<TH1 *> > &hists)
{
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists[ig].size(); ++ia)
        {
            hists[ig][ia]->Reset();
        }
    }
}

void Calibration::deleteHists(std::vector<std::vector<TH1 *> > &hists)
{
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists[ig].size(); ++ia)
        {
            delete hists[ig][ia];
            hists[ig][ia] = nullptr;
        }
    }
    hists.clear();
}

void Calibration::fillHistsAsync(const std::vector<std::vector<TH1 *> > &hists, std::function<double(const dec_ev_t &)> f)
{
    std::vector<std::future<void>> futures;
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists[ig].size(); ++ia)
        {
            futures.emplace_back(std::async(std::launch::async, [this, &hists, &f] (u_int8_t g, u_int8_t a) {
                auto sE{selectedEvents(g, a)};
                fillHist(sE, hists[g][a], f);
            }, ig, ia));
        }
    }

    for (size_t i{0}; i < futures.size(); ++i)
    {
        futures[i].get();
    }
}

void Calibration::processTimeStamp()
{

}

void Calibration::processTime()
{
    std::vector<std::vector<TH1 *>> hists(nGamma_);
    prepareHists("histTime", 400, -100, 100, hists);

    fillHistsAsync(hists, std::bind(&Calibration::valueTime, this, std::placeholders::_1));

    calculateTimePeaksPos(hists);

    const std::string psName{"time.ps"};
    drawHistsToFile(psName, hists);

    clearHists(hists);
    fillHistsAsync(hists, std::bind(&Calibration::valueTime, this, std::placeholders::_1));



    deleteHists(hists);
}

void Calibration::processGammaAmp()
{
    std::vector<std::vector<TH1 *>> hists(nGamma_);
    prepareHists("histGammaAmp", 640, 0, 4e3, hists);

    fillHistsAsync(hists, std::bind(&Calibration::valueGammaAmp, this, std::placeholders::_1));

    const std::string psName{"gamma_amp.ps"};
//    drawHistsToFile(psName, hists);

    deleteHists(hists);
}
