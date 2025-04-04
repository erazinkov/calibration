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

    stor_nd_t nd;

    auto c{false};
    auto spillNumber{0};
    long long int prevTs{0};

    size_t index{0};

    auto isIntegerOverflow = [](long long int currentTs, long long int prevTs, long long int limit = 3'000'000'000){
        return std::abs(currentTs - prevTs) > limit;
    };

    while (ifs_)
    {
        ifs_ >> hdr;

        if (hdr.id == STOR_ID_ND && hdr.size > sizeof(stor_packet_hdr_t))
        {
            if (events_.size())
            {
                long long int d{events_[events_.size() - 1].ts - events_[index].ts};
                long long int t0{nd.time * 1'000'000 - d * 10};
//                std::cout << nd.time * 1'000'000 << " " << t0 * 10 << " " << nd.time * 1'000'000 + t0 * 10 << std::endl;
//                const std::chrono::system_clock::time_point tp{std::chrono::nanoseconds(t00)};
//                const std::time_t t_c = std::chrono::system_clock::to_time_t(tp);
//                std::cout << std::put_time(std::localtime(&t_c), "%F %T") << std::endl;
                for (size_t i{index}; i < events_.size(); ++i)
                {
                    events_[i].time = t0 + events_[i].ts * 10 - events_[index].ts * 10;
                }
                index = events_.size() - 1;
            }

            ifs_ >> nd;
            continue;
        }
        if (hdr.id == STOR_ID_CMAP && hdr.size > sizeof(stor_packet_hdr_t))
        {
            ifs_ >> cmap;
            c = pre_.isCorrect(cmap.map);
            if (spillNumber++ == 1)
            {
                break;
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
                long long int currentTs{static_cast<long long int>(ev.ts)};
                event.ts = currentTs;
                while (isIntegerOverflow(event.ts, prevTs) && events_.size()) {
                    event.ts += UINT32_MAX;
                }
                prevTs = event.ts;
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

std::vector<long> Decoder::positionsOfCMAPHeaders()
{
    std::vector<long> pos{};
    ifs_.open(fileName_, std::ios::in | std::ios::binary);
    if (!ifs_.is_open())
    {
        std::clog << "Can't open file" << std::endl;
        return pos;
    }

    stor_packet_hdr_t hdr;
    adcm_cmap_t cmap;
    adcm_counters_t counters;

    auto c{false};
    long currentPosition{0};
    while (ifs_)
    {
        ifs_ >> hdr;
        currentPosition += sizeof(stor_packet_hdr_t);
        if (hdr.id == STOR_ID_CMAP && hdr.size > sizeof(stor_packet_hdr_t))
        {
            currentPosition = ifs_.tellg();
            ifs_ >> cmap;
            c = pre_.isCorrect(cmap.map);
            if (c)
            {
                currentPosition -= sizeof(stor_packet_hdr_t);
                pos.push_back(currentPosition);
            }
            continue;
        }
        if (hdr.id == STOR_ID_EVNT && hdr.size > sizeof(stor_packet_hdr_t))
        {
            hdr.size -= sizeof(stor_packet_hdr_t);
            ifs_.ignore(hdr.size);
            continue;
        }
        if (hdr.id == STOR_ID_CNTR && hdr.size > sizeof(stor_packet_hdr_t))
        {

            hdr.size -= sizeof(stor_packet_hdr_t);
            ifs_.ignore(hdr.size);
            continue;
        }
        ifs_.seekg(1 - static_cast<long long>(sizeof(stor_packet_hdr_t)), std::ios_base::cur);
    }
    ifs_.close();

    return pos;
}

std::vector<long> Decoder::positionsOfNDHeaders()
{
    std::vector<long> pos{};
    ifs_.open(fileName_, std::ios::in | std::ios::binary);
    if (!ifs_.is_open())
    {
        std::clog << "Can't open file" << std::endl;
        return pos;
    }

    stor_packet_hdr_t hdr;
    stor_nd_t nd;
    adcm_counters_t counters;

    long currentPosition{0};
    while (ifs_)
    {
        ifs_ >> hdr;
        currentPosition += sizeof(stor_packet_hdr_t);
        if (hdr.id == STOR_ID_ND && hdr.size > sizeof(stor_packet_hdr_t))
        {
            currentPosition = ifs_.tellg();
            ifs_ >> nd;
            currentPosition -= sizeof(stor_packet_hdr_t);
            pos.push_back(currentPosition);
            continue;
        }
        if (hdr.id == STOR_ID_CMAP && hdr.size > sizeof(stor_packet_hdr_t))
        {
            hdr.size -= sizeof(stor_packet_hdr_t);
            ifs_.ignore(hdr.size);
            continue;
        }
        if (hdr.id == STOR_ID_EVNT && hdr.size > sizeof(stor_packet_hdr_t))
        {
            hdr.size -= sizeof(stor_packet_hdr_t);
            ifs_.ignore(hdr.size);
            continue;
        }
        if (hdr.id == STOR_ID_CNTR && hdr.size > sizeof(stor_packet_hdr_t))
        {

            hdr.size -= sizeof(stor_packet_hdr_t);
            ifs_.ignore(hdr.size);
            continue;
        }
        ifs_.seekg(1 - static_cast<long long>(sizeof(stor_packet_hdr_t)), std::ios_base::cur);
    }
    ifs_.close();

    return pos;
}

