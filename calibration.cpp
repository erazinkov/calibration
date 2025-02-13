#include "calibration.h"

#include <TCanvas.h>
#include <TError.h>

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

double Calibration::valueTime(const dec_ev_t &event)
{
    return static_cast<double>(event.tdc) - _timePeaksPos[event.g.index][event.a.index];
}

double Calibration::valueGammaAmp(const dec_ev_t &event)
{
    return static_cast<double>(event.g.amp);
}

void Calibration::fillHist(const std::vector<dec_ev_t> &events, TH1 *h, double(Calibration::*f)(const dec_ev_t &event))
{
    for (const auto & item : events)
    {
        auto v{(this->*f)(item)};
        h->Fill(v);
    }
}

void Calibration::drawHistsToFile(const std::string &psName, std::vector<std::vector<TH1 *> > hists)
{
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c->Print((psName + '[').c_str());
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        c->Divide(3, 3);
        for (size_t ia{0}; ia <  hists[ig].size(); ++ia)
        {
            c->cd(static_cast<int>(ia) + 1);
            hists[ig][ia]->Draw();
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
        for (size_t ia{0}; ia <  hists[ig].size(); ++ia)
        {
            delete hists[ig][ia];
            hists[ig][ia] = nullptr;
        }
    }
    hists.clear();
}

void Calibration::fillHistsAsync(std::vector<std::vector<TH1 *> > hists, double (Calibration::*f)(const dec_ev_t &))
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

void Calibration::processTime()
{
    std::vector<std::vector<TH1 *>> hists(_nGamma);
    prepareHists("histTime", 400, -100, 100, hists);

    double(Calibration::*f)(const dec_ev_t &event);
    f = &Calibration::valueTime;

    fillHistsAsync(hists, f);

    const std::string psName{"time.ps"};
    drawHistsToFile(psName, hists);

    clearHists(hists);
}

void Calibration::processGammaAmp()
{
    std::vector<std::vector<TH1 *>> hists(_nGamma);
    prepareHists("histGammaAmp", 640, 0, 4e3, hists);

    double(Calibration::*f)(const dec_ev_t &event);
    f = &Calibration::valueGammaAmp;

    fillHistsAsync(hists, f);

    const std::string psName{"gamma_amp.ps"};
    drawHistsToFile(psName, hists);

    clearHists(hists);
}
