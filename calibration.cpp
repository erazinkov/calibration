#include "calibration.h"

#include <TCanvas.h>
#include <TError.h>

#include <TF1.h>
#include <TFile.h>

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
    processGammaAmp();
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

void Calibration::calculateTimePeaksPos(const std::vector<std::vector<TH1 *> > &hists)
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

void Calibration::fillHist(const std::vector<dec_ev_t> &events, TH1 *h, FillOptions &fillOptions)
{
    using Value = FillOptions::Value;

    for (const auto & item : events)
    {
        switch (fillOptions.value()) {
            case Value::ENERGY:
            {
                h->Fill(static_cast<double>(item.g.amp));
                break;
            }
            case Value::TIME:
                h->Fill(static_cast<double>(item.tdc));
                break;
        }
    }
}

void Calibration::fillHistsAsync(const std::vector<std::vector<TH1 *> > &hists, FillOptions &fillOptions)
{
    std::vector<std::future<void>> futures;
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists[ig].size(); ++ia)
        {
            futures.emplace_back(std::async(std::launch::async, [this, &hists, &fillOptions] (u_int8_t g, u_int8_t a) {
                auto sE{selectedEvents(g, a)};
                fillHist(sE, hists[g][a], fillOptions);
            }, ig, ia));
        }
    }

    for (size_t i{0}; i < futures.size(); ++i)
    {
        futures[i].get();
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
    for (size_t ig{0}; ig < _nGamma; ++ig)
    {
        for (size_t ia{0}; ia <  _nAlpha; ++ia)
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
        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
        {
            hists[ig][ia]->Reset();
        }
    }
}

void Calibration::deleteHists(std::vector<std::vector<TH1 *> > &hists)
{
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
        {
            delete hists[ig][ia];
            hists[ig][ia] = nullptr;
        }
    }
    hists.clear();
}

void Calibration::processTimeStamp()
{

}

void Calibration::processTime()
{
    std::vector<std::vector<TH1 *>> hists(_nGamma);
    prepareHists("histTime", 400, -100, 100, hists);

    using Value = FillOptions::Value;

    FillOptions fillOptions(Value::TIME);
    fillHistsAsync(hists, fillOptions);

    calculateTimePeaksPos(hists);

    const std::string psName{"time.ps"};
    drawHistsToFile(psName, hists);

    clearHists(hists);

    deleteHists(hists);
}

void Calibration::processGammaAmp()
{
    std::vector<std::vector<TH1 *>> hists(_nGamma);
    prepareHists("histGammaCh", 640, 0, 4e3, hists);

    using Type = FillOptions::Type;

    FillOptions fillOptions(Type::ENERGY);
    fillHistsAsync(hists, fillOptions);

    const std::string psName{"gamma_ch.ps"};
    drawHistsToFile(psName, hists);

    clearHists(hists);
    deleteHists(hists);
}
