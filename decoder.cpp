#include "decoder.h"

#include <iostream>
#include <bits/stdc++.h>

#include "progressbar.h"

Decoder::Decoder(const std::string &fileName, const ChannelMap &pre)
    : fileName_{fileName} , _map{pre}
{
    process();
}

std::vector<dec_ev_t> &Decoder::events()
{
    return events_;
}

dec_cnt_t &Decoder::counters()
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

    ifs_.seekg(0, std::ios::end);
    u_int32_t size{static_cast<u_int32_t>(ifs_.tellg())};
    ifs_.seekg(0);

    auto number{_map.getIdxsByType(Channel::ALPHA).size() + _map.getIdxsByType(Channel::GAMMA).size()};
    counters_.rawhits.resize(number);

    events_.clear();

    stor_packet_hdr_t hdr;
    stor_ev_hdr_t ev;
    adcm_cmap_t cmap;
    adcm_counters_t counters;

    auto c{false};
    u_int32_t currentPosition{0};
    double prevTs{0};

    auto isIntegerOverflow = [](double currentTs, double prevTs, double limit = 3'000'000'000){
        return std::abs(currentTs - prevTs) > limit;
    };

    while (ifs_)
    {
        ifs_ >> hdr;
        if (hdr.id == STOR_ID_CMAP && hdr.size > sizeof(stor_packet_hdr_t))
        {
            currentPosition = static_cast<u_int32_t>(ifs_.tellg());
            ifs_ >> cmap;
            c = _map.isCorrect(cmap.map);
            if (c)
            {
                currentPosition -= sizeof(stor_packet_hdr_t);
                ProgressBar<u_int32_t>::show(currentPosition, size);
            }
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


            auto idxGamma{_map.getIdxByHardwareIdx(g->ch)};
            auto idxAlpha{_map.getIdxByHardwareIdx(a->ch)};
            if (idxGamma.has_value() && idxAlpha.has_value())
            {
                dec_ev_t event;
                event.g.index = idxGamma.value();
                event.g.amp = g->a;
                event.a.index = idxAlpha.value();
                event.a.amp = a->a;
                event.tdc = g->t - a->t;
                double currentTs{static_cast<double>(ev.ts)};
                event.ts = currentTs;
                if (isIntegerOverflow(event.ts, prevTs) && events_.size()) {
                    event.ts += UINT32_MAX;
                }
                prevTs = event.ts;
                events_.push_back(event);
            }
//            if (g->ch < _map.map().size() && a->ch < _map.map().size())
//            {
//                dec_ev_t event;
//                auto idxGamma{_map.map().at(g->ch).softwareIndex()};
//                auto idxAlpha{_map.map().at(a->ch).softwareIndex()};
//                event.g.index = idxGamma;
//                event.g.amp = g->a;
//                event.a.index = idxAlpha;
//                event.a.amp = a->a;
//                event.tdc = g->t - a->t;
//                double currentTs{static_cast<double>(ev.ts)};
//                event.ts = currentTs;
//                if (isIntegerOverflow(event.ts, prevTs) && events_.size()) {
//                    event.ts += UINT32_MAX;
//                }
//                prevTs = event.ts;
//                events_.push_back(event);
//            }
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
                continue;
            }
            ifs_ >> counters;

            if (!counters.n)
            {
                continue;
            }
//            for (size_t i{0}; i < _map.map().size(); ++i)
//            {
//                auto idx{_map.map().at(i).softwareIndex()};
//                auto type{_map.map().at(i).type()};
//                if (type == Channel::UNKNOWN)
//                {
//                    continue;
//                }
//                counters_.rawhits.at(idx) += counters.rawhits.at(i);
//            }
            counters_.time += counters.time;
            ifs_.ignore(hdr.size
                       - sizeof(stor_packet_hdr_t)
                       - (sizeof(counters.n) + sizeof(counters.time) + sizeof(*counters.rawhits.cbegin()) * counters.n));
            continue;
        }
        ifs_.seekg(1 - static_cast<long long>(sizeof(stor_packet_hdr_t)), std::ios_base::cur);
    }
    ifs_.close();
}


