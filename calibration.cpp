#include "calibration.h"

#include <TCanvas.h>
#include <TError.h>

#include <TF1.h>
#include <TFile.h>

#include "spinner.h"

#include "peakfinder.h"

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

    process();
}

void Calibration::process()
{
//    processTimeStamp();
   processTime();
    processGammaCh();
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

void Calibration::calculateTimePeaksPos(std::vector<std::vector<TH1 *> > &hists)
{
    gErrorIgnoreLevel = 3'000;
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
        {
            _timePeaksPos.at(ig).at(ia) = calculateTimePeakPos(hists.at(ig).at(ia));
            if (ig == 0 && ia == 0)
            {
                std::cout << _timePeaksPos.at(ig).at(ia) << std::endl;
            }
        }
    }
    gErrorIgnoreLevel = 0;
}

double Calibration::calculateTimePeakPos(TH1 *hist) const
{
    hist->Rebin();
    auto timePeakPos{0.0};

    auto binMax{hist->GetMaximumBin()};
    auto xMax{hist->GetBinCenter(hist->GetBin(binMax))};
    auto rcAmp{hist->GetBinContent(hist->GetXaxis()->FindBin(xMax - 25.0))};
    auto obPeakAmp{hist->GetBinContent(binMax) - rcAmp};
    auto snPeakAmp{0.5 * obPeakAmp};
    TF1 *f{new TF1("f", _timePeakFitFunctionObject, xMax - 15.0, xMax + 25.0, 11)};

    f->SetParameter(0, obPeakAmp);
    f->SetParameter(1, xMax);
    f->SetParameter(2, 0.5 * ( 1.5 + 3.0 ));
    f->SetParameter(6, 0.05 * obPeakAmp);
    f->SetParameter(7, -10.0);
    f->SetParameter(8, 2.5);
    f->SetParameter(9, rcAmp);
    f->FixParameter(10, 0.0);

    f->SetParLimits(1, 0.9 * xMax, 1.1 * xMax);
    f->SetParLimits(2, 1.5, 3.0);
    f->SetParLimits(3, 0.0, snPeakAmp);
    f->SetParLimits(4, 5.0, 20.0);
    f->SetParLimits(5, 2.0, 7.0);
    f->SetParLimits(6, 0.0, 0.25 * obPeakAmp);
    f->SetParLimits(7, -15.0, -7.5);
    f->SetParLimits(8, 2.25, 2.75);

    hist->GetXaxis()->SetRangeUser(f->GetParameter(1) - 40.0, f->GetParameter(1) + 25.0);

    hist->Fit(f, "RQ");

    auto ff = [](double *x, double *par){
        double arg{0};
        if (par[2] != 0.0)
        {
            arg = ( x[0] - par[1] ) / par[2];
        }
        double fitval{par[0] * TMath::Exp(-0.5 * arg * arg) + par[3] + par[4] * x[0]};
        return fitval;
    };

    TF1 *fOb{new TF1("fOb", ff, xMax - 15.0, xMax + 25.0, 5)};
    fOb->SetParameters(f->GetParameter(0),
                       f->GetParameter(1),
                       f->GetParameter(2),
                       f->GetParameter(9),
                       f->GetParameter(10));
    fOb->SetLineColor(kGreen);
    TF1 *fB{new TF1("fB", ff, xMax - 15.0, xMax + 25.0, 5)};
    fB->SetParameters(f->GetParameter(6),
                       f->GetParameter(1) + f->GetParameter(7),
                       f->GetParameter(8),
                       f->GetParameter(9),
                       f->GetParameter(10));
    fB->SetLineColor(kMagenta);
    TF1 *fSn{new TF1("fSn", ff, xMax - 15.0, xMax + 25.0, 5)};
    fSn->SetParameters(f->GetParameter(3),
                       f->GetParameter(1) + f->GetParameter(4),
                       f->GetParameter(5),
                       f->GetParameter(9),
                       f->GetParameter(10));
    fSn->SetLineColor(kBlue);

    hist->GetListOfFunctions()->Add(fOb);
    hist->GetListOfFunctions()->Add(fB);
    hist->GetListOfFunctions()->Add(fSn);

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
    std::vector<std::vector<TH1 *>> hists(_nGamma);
    for (size_t i{0}; i < hists.size(); ++i)
    {
        hists.at(i).resize(_nAlpha, nullptr);
    }
    prepareHists("histTime", BINS_TIME, XLOW_TIME, XUP_TIME, hists);

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

    std::vector<std::future<void>> futures;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j)
        {
            futures.emplace_back(std::async(std::launch::async, [this] (uint8_t g, uint8_t a, TH1 *hSg, TH1 *hBg, TH1 *hRc) {
                auto sE{selectedEvents(g, a)};
                auto tSgMin{_timePeaksPos.at(g).at(a) - 3.0};
                auto tSgMax{_timePeaksPos.at(g).at(a) + 3.0};
                fillHistChannel(sE, hSg, tSgMin, tSgMax, false);
                fillHistChannel(sE, hRc, tSgMin - 1.0, tSgMax + 1.0, true);
                auto tBgMin{_timePeaksPos.at(g).at(a) - 30.0};
                auto tBgMax{_timePeaksPos.at(g).at(a) - 20.0};
                fillHistChannel(sE, hBg, tBgMin, tBgMax, false);
            }, i, j, histsSg.at(i).at(j), histsBg.at(i).at(j), histsRc.at(i).at(j)));
        }
    }

    for (size_t i{0}; i < futures.size(); ++i)
    {
        futures[i].get();
    }

    futures.clear();

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

    PeakFinder peakFinder;
    peakFinder.process(histsSgGamma, histsRcGamma);

    auto p{peakFinder.getPar()};
    for (size_t i{0}; i < p.size(); ++i)
    {
        for (size_t j{0}; j < p.at(i).size(); ++j)
        {
            _par.at(i).push_back(p.at(i).at(j));
        }
    }

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

    std::vector<std::future<void>> futures;
    for (size_t i{0}; i < histsSg.size(); ++i)
    {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j)
        {
            futures.emplace_back(std::async(std::launch::async, [this] (uint8_t g, uint8_t a, TH1 *hSg, TH1 *hBg) {
                auto sE{selectedEvents(g, a)};
                auto tSgMin{_timePeaksPos.at(g).at(a) - 3.0};
                auto tSgMax{_timePeaksPos.at(g).at(a) + 3.0};
                TF1 f("f", "pol3", XLOW_ENERGY, XUP_ENERGY);
                f.SetParameters(&_par.at(g).at(0));
                fillHistEnergy(sE, hSg, tSgMin, tSgMax, false, f);
                auto tBgMin{_timePeaksPos.at(g).at(a) - 30.0};
                auto tBgMax{_timePeaksPos.at(g).at(a) - 20.0};
                fillHistEnergy(sE, hBg, tBgMin, tBgMax, false, f);
            }, i, j, histsSg.at(i).at(j), histsBg.at(i).at(j)));
        }
    }

    for (size_t i{0}; i < futures.size(); ++i)
    {
        futures[i].get();
    }

    futures.clear();

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
