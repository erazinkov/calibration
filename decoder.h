#ifndef DECODER_H
#define DECODER_H

#include "adcm_df.h"
#include "channelmap.h"

class Decoder
{
public:
    Decoder(const std::string &, const ChannelMap &);
    dec_cnt_t & counters();
    void process();
    const std::map<std::string, u_int64_t> &pulses() const;

    const std::vector<dec_ev_2p_t> &events_2p() const;

    const std::vector<dec_ev_3p_t> &events_3p() const;

private:

    std::string fileName_;
    std::ifstream ifs_;
    ChannelMap map_;
    std::vector<dec_ev_2p_t> events_2p_;
    std::vector<dec_ev_3p_t> events_3p_;
    dec_cnt_t counters_;
    std::map<std::string, u_int64_t> pulses_;

};


#endif // DECODER_H
