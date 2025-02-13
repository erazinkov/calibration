#ifndef DECODER_H
#define DECODER_H

#include "adcm_df.h"
#include "channelmap.h"

class Decoder
{
public:
    Decoder(const std::string &, const ChannelMap &);
    std::vector<dec_ev_t> & events();
    std::vector<dec_cnt_t> & counters();
    void process();
private:

    std::string fileName_;
    std::ifstream ifs_;
    ChannelMap pre_;
    std::vector<dec_ev_t> events_;
    std::vector<dec_cnt_t> counters_;

};


#endif // DECODER_H
