#ifndef DECODER_H
#define DECODER_H

#include "adcm_df.h"
#include "channelmap.h"

class Decoder
{
public:
    Decoder(const std::string &, const ChannelMap &);
    std::vector<dec_ev_t> & events();
    dec_cnt_t & counters();
    void process();
    const std::vector<dec_ev_t_3_p> &events_3_p() const;

private:

    std::string fileName_;
    std::ifstream ifs_;
    ChannelMap map_;
    std::vector<dec_ev_t> events_;
    dec_cnt_t counters_;

    std::vector<dec_ev_t_3_p> events_3_p_;

};


#endif // DECODER_H
