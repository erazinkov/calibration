#ifndef DECODER_H
#define DECODER_H

#include "adcm_df.h"
#include "channelmap.h"

class Decoder
{
public:
    Decoder(const ChannelMap &);
    std::vector<dec_ev_t> & events();
    void process(const std::string &);
    void process(const std::string &, const long long int &, const std::pair<long long int, long long int> &);

    std::vector<long> positionsOfCMAPHeaders(const std::string &);
    std::vector<long> positionsOfNDHeaders(const std::string &);
private:
    std::ifstream ifs_;
    ChannelMap pre_;
    std::vector<dec_ev_t> events_;
    dec_cnt_t counters_;
};


#endif // DECODER_H
