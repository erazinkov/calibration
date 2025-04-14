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
    processTimeStamp();
//    processTime();
//    processGammaAmp();
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

long long int Calibration::valueTimeStamp(const dec_ev_t &event)
{
    return event.ts;
}

double Calibration::valueTime(const dec_ev_t &event)
{
    return static_cast<double>(event.tdc) - _timePeaksPos[event.g.index][event.a.index];
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
            _timePeaksPos[ig][ia] = calculateTimePeakPos(hists[ig][ia]);
        }
    }
    gErrorIgnoreLevel = 0;
}

double Calibration::calculateTimePeakPos(TH1 *hist)
{
    auto timePeakPos{0.0};
    auto h{hist};
    auto binMax{h->GetMaximumBin()};
    auto xMax{h->GetBinCenter(h->GetBin(binMax))};
    auto rcAmp{h->GetBinContent(h->GetXaxis()->FindBin(xMax - 25.0))};
    auto peakAmp{h->GetBinContent(binMax) - rcAmp};
    TF1 *f = new TF1("f", _timePeakFitFunctionObject, xMax - 25.0, xMax + 25.0, 5);
    f->SetParameters(peakAmp, xMax, 5.0, rcAmp, 0.0);
    h->Fit(f, "RQ");
    timePeakPos = f->GetParameter(1);
    delete f;
    f = nullptr;
    return timePeakPos;
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

void Calibration::fillHistsAsync(const std::vector<std::vector<TH1 *> > &hists, double (Calibration::*f)(const dec_ev_t &))
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

void Calibration::fillHist(TH1 *hist, timespec (Calibration::*f)(const dec_ev_t &))
{
//    auto bin{0};
//    for (size_t i{0}; i < _events.size() - 1; ++i)
//    {
//        auto vNext{(this->*f)(_events[i + 1])};
//        auto vPrev{(this->*f)(_events[i])};
//        hist->SetBinContent(++bin, vNext - vPrev);
//    }

    auto bin{0};
    for (const auto & item : _events)
    {
        bin++;
        if (bin > 10)
        {
            break;
        }
        auto v{(this->*f)(item)};

        const std::chrono::system_clock::time_point tp{std::chrono::seconds(v.tv_sec)};
        const std::time_t t_c = std::chrono::system_clock::to_time_t(tp);
//        std::cout << std::put_time(std::localtime(&t_c), "%F %T") << "." << v.tv_nsec << std::endl;
//        hist->SetBinContent(++bin, static_cast<double>(v));
    }
}

void Calibration::fillHist(TH1 *hist, long long int (Calibration::*f)(const dec_ev_t &))
{
    auto bin{0};
    for (const auto & item : _events)
    {
        auto v{(this->*f)(item)};
        if (bin < 5)
        {
            std::chrono::system_clock::time_point tp{std::chrono::nanoseconds(v)};
            std::time_t t_c = std::chrono::system_clock::to_time_t(tp);
            const std::chrono::duration<double> tse = tp.time_since_epoch();
            std::chrono::seconds::rep nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(tse).count() % 1'000'000'000;
            auto localTime{*std::localtime(&t_c)};
            std::cout << (1900 + localTime.tm_year) << '-'
                    << std::setfill('0') << std::setw(2) << (localTime.tm_mon + 1) << '-'
                    << std::setfill('0') << std::setw(2) << localTime.tm_mday << ' '
                    << std::setfill('0') << std::setw(2) << localTime.tm_hour << ':'
                    << std::setfill('0') << std::setw(2) << localTime.tm_min << ':'
                    << std::setfill('0') << std::setw(2) << localTime.tm_sec << '.'
                       << std::setfill('0') << std::setw(9) << nanoseconds
                    << std::endl;
        }
        hist->SetBinContent(++bin, static_cast<double>(v));
    }
}

void Calibration::processTimeStamp()
{
    TH1 *hist{new TH1D("histTimeStamp", "histTimeStamp", 10'500'000, 0, 10'500'000)};

    long long int(Calibration::*f)(const dec_ev_t &event);
    f = &Calibration::valueTimeStamp;

    fillHist(hist, f);

    const std::string psName{"time_stamp.ps"};

    std::unique_ptr<TFile> myFile( TFile::Open("time_stamp.root", "RECREATE") );
    myFile->WriteObject(hist, hist->GetName());
    delete hist;
}

void Calibration::processTime()
{
    std::vector<std::vector<TH1 *>> hists(_nGamma);
    prepareHists("histTime", 400, -100, 100, hists);

    double(Calibration::*f)(const dec_ev_t &event);
    f = &Calibration::valueTime;

    fillHistsAsync(hists, f);

//    calculateTimePeaksPos(hists);

//    clearHists(hists);
//    fillHistsAsync(hists, f);

    const std::string psName{"time.ps"};
    drawHistsToFile(psName, hists);

    deleteHists(hists);
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

    deleteHists(hists);
}
