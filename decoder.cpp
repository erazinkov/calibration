#include "decoder.h"

#include <iostream>
#include <bits/stdc++.h>

Decoder::Decoder(const std::string &fileName, const ChannelMap &pre)
    : fileName_{fileName} , pre_{pre}
{
    process();
}

std::vector<dec_ev_t> &Decoder::events()
{
    return events_;
}

std::vector<dec_cnt_t> &Decoder::counters()
{
    return counters_;
}

void Decoder::process()
{
    ifs_.open(fileName_, std::ios::in | std::ios::binary);
    if (!ifs_.is_open())
    {
        std::clog << "Can't open file" << std::endl;
        return;
    }

    auto number{pre_.numberOfChannelsAlpha()};
    counters_.resize(number);

    events_.clear();

    stor_packet_hdr_t hdr;
    stor_ev_hdr_t ev;
    adcm_cmap_t cmap;
    adcm_counters_t counters;

    auto c{false};

    while (ifs_)
    {
        ifs_ >> hdr;

        if (hdr.id == STOR_ID_CMAP && hdr.size > sizeof(stor_packet_hdr_t))
        {
            ifs_ >> cmap;
            c = pre_.isCorrect(cmap.map);
            continue;
        }
        if (hdr.id == STOR_ID_EVNT && hdr.size > sizeof(stor_packet_hdr_t))
        {
            if (!c)
            {
                hdr.size -= sizeof(stor_packet_hdr_t);
                ifs_.ignore(hdr.size);
                continue;
            }
            ifs_ >> ev;
            if (ev.np != 2)
            {
                hdr.size -= sizeof(stor_packet_hdr_t);
                hdr.size -= sizeof(stor_ev_hdr_t);
                ifs_.ignore(hdr.size);
                continue;
            }
            stor_puls_t *g = new stor_puls_t();
            stor_puls_t *a = new stor_puls_t();
            ifs_ >> *g >> *a;
            if (g->ch < pre_.map().size() && a->ch < pre_.map().size())
            {
                dec_ev_t event;
                auto numberGamma{pre_.numberByChannel(g->ch)};
                auto numberAlpha{pre_.numberByChannel(a->ch)};
                event.g.index = numberGamma;
                event.g.amp = g->a;
                event.a.index = numberAlpha;
                event.a.amp = g->a;
                event.tdc = g->t - a->t;
                event.ts = ev.ts;
                events_.push_back(event);
            }
            delete g;
            delete a;

            continue;
        }
        if (hdr.id == STOR_ID_CNTR && hdr.size > sizeof(stor_packet_hdr_t))
        {
            if (!c)
            {
                hdr.size -= sizeof(stor_packet_hdr_t);
                ifs_.ignore(hdr.size);
            }
            ifs_ >> counters;
            for (size_t i{0}; i < pre_.map().size(); ++i)
            {
                auto number{pre_.numberByChannel(i)};
                auto type{pre_.typeByChannel(i)};
                switch (type)
                {
                    case ALPHA:
                    {
                        counters_[number].rawhits += counters.rawhits[i];
                        counters_[number].time += counters.time;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            continue;
        }
        ifs_.seekg(1 - static_cast<long long>(sizeof(stor_packet_hdr_t)), std::ios_base::cur);
    }
    ifs_.close();
}


