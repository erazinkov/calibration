#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <functional>

#include <TH1.h>
#include <TMath.h>

#include "adcm_df.h"
#include "channelmap.h"

#include "timepeaksfinder.h"

class Calibration
{
public:
    Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events);
    ~Calibration();
    void process();

    static inline constexpr int BINS_TIME{400};
    static inline constexpr int BINS_CHANNEL{400};
    static inline constexpr int BINS_ENERGY{640};

    static inline constexpr double XLOW_TIME{-100.0};
    static inline constexpr double XLOW_CHANNEL{0.0};
    static inline constexpr double XLOW_ENERGY{0.0};

    static inline constexpr double XUP_TIME{100.0};
    static inline constexpr double XUP_CHANNEL{4.0e3};
    static inline constexpr double XUP_ENERGY{8.0e3};

private:
    TimePeaksFinder *_timePeaksFinder;
    const ChannelMap _map;
    const std::vector<dec_ev_t> _events;

    std::vector<dec_ev_t> selectedEvents(uint8_t ig, u_int8_t ia);

    void fillHistTime(const std::vector<dec_ev_t> &, TH1 *, double);
    void fillHistChannel(const std::vector<dec_ev_t> &events, TH1 *h, double min, double max, bool exclude);
    void fillHistEnergy(const std::vector<dec_ev_t> &events, TH1 *h, double min, double max, bool exclude, TF1 f);

    void drawHistsToFile(const std::string &psName, const std::vector<std::vector<TH1 *> > &hists) const;
    void prepareHists(const std::string &histName,
                      int nBinsX,
                      double xLow,
                      double xUp,
                      std::vector<std::vector<TH1 *>> &hists);
    void prepareHists(const std::string &histName,
                      int nBinsX,
                      double xLow,
                      double xUp,
                      std::vector<TH1 *> &hists);
    void clearHists(std::vector<std::vector<TH1 *>> &hists);
    void clearHists(std::vector<TH1 *> &hists);
    void deleteHists(std::vector<std::vector<TH1 *>> &hists);
    void deleteHists(std::vector<TH1 *> &hists);

    void processTimeStamp();
    void processTime();
    void processGammaCh();
    void processGammaEnergy();

    std::vector<std::vector<double>> _timePeaksPos;
    std::vector<std::vector<double>> _par;
    unsigned long _nGamma;
    unsigned long _nAlpha;

    class AmpPeakFitFunctionObject
    {
    public:
        AmpPeakFitFunctionObject(){}

        double operator() (double *x, double *par) {
           double arg{0};
           if (par[2] != 0.0)
           {
               arg = ( x[0] - par[1] ) / par[2];
           }
           double fitval{par[0] * TMath::Exp(-0.5 * arg * arg) + par[3] + par[4] * x[0]};
           return fitval;
       }
    };
    AmpPeakFitFunctionObject _ampPeakFitFunctionObject;

    template<typename  Iterator>
    struct fill_hist_block
    {
        void operator()(Iterator first, Iterator last)
        {
            for (auto it{first}; it != last; ++it)
            {
                (*it)();
            }
        }
    };

    template<typename Iterator>
    void fill_hist_async(Iterator first, Iterator last)
    {
        unsigned long const length{static_cast<unsigned long const>(std::distance(first, last))};
        if (!length)
        {
            return;
        }
        unsigned long const min_per_thread{10};
        unsigned long const max_threads{(length + min_per_thread - 1) / min_per_thread};
        unsigned long const hardware_threads{std::thread::hardware_concurrency()};
        unsigned long const num_threads{std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads)};
        unsigned long const block_size{length / num_threads};
        std::vector<std::thread> threads(num_threads - 1);
        Iterator block_start{first};
        for(unsigned long i{0}; i < (num_threads - 1); ++i)
        {
            Iterator block_end{block_start};
            std::advance(block_end,block_size);
            threads.at(i) = std::thread(fill_hist_block<Iterator>(), block_start, block_end);
            block_start = block_end;
        }

        fill_hist_block<Iterator>()(block_start, last);

        for(auto& entry: threads) {
            entry.join();
        }
    }

};

#endif // CALIBRATION_H
