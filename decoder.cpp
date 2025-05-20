#include "decoder.h"

Decoder::Decoder(const ChannelMap &pre)
    : pre_{pre}
{
}

std::vector<dec_ev_t> &Decoder::events()
{
    return events_;
}

void Decoder::process(const std::string &fileName)
{
    ifs_.open(fileName, std::ios::in | std::ios::binary);
    if (!ifs_.is_open())
    {
        std::clog << "Can't open file" << std::endl;
        return;
    }

    counters_.rawhits.resize(pre_.numberOfChannelsAlpha());

    events_.clear();

    stor_packet_hdr_t hdr;
    stor_ev_hdr_t ev;
    adcm_cmap_t cmap;
    adcm_counters_t counters;

    stor_nd_t nd;


    auto c{false};

    auto isIntegerOverflow = [](long long int currentTs, long long int prevTs, long long int limit = 3'000'000'000){
        return std::abs(currentTs - prevTs) > limit;
    };

    std::vector<dec_ev_t> spillEvents;

    while (ifs_)
    {
        ifs_ >> hdr;

        if (hdr.id == STOR_ID_ND && hdr.size > sizeof(stor_packet_hdr_t))
        {
            ifs_ >> nd;
            continue;
        }
        if (hdr.id == STOR_ID_CMAP && hdr.size > sizeof(stor_packet_hdr_t))
        {
            ifs_ >> cmap;
            c = pre_.isCorrect(cmap.map);

            spillEvents.clear();
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
                spillEvents.push_back(event);
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
                continue;
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
                        counters_.rawhits[number] += counters.rawhits[i];
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            counters_.time += counters.time;
            if (spillEvents.size())
            {

                for (size_t i{1}; i < spillEvents.size(); ++i)
                {
                    if (isIntegerOverflow(spillEvents[i].ts, spillEvents[0].ts) && spillEvents.size()) {
                        spillEvents[i].ts += UINT32_MAX;
                    }
                }
                auto tsOffset{spillEvents[0].ts};
                for (size_t i{0}; i < spillEvents.size(); ++i)
                {
                    spillEvents[i].ts -= tsOffset;
                }
                for (size_t i{0}; i < spillEvents.size(); ++i)
                {
                    auto dateInNanoSec{nd.time + spillEvents[i].ts * 10 - spillEvents[spillEvents.size() - 1].ts * 10};
                    spillEvents[i].ts = dateInNanoSec;
                }

                events_.insert(events_.cend(), spillEvents.cbegin(), spillEvents.cend());
                std::cout << events_.size() << std::endl;
            }
            continue;
        }
        ifs_.seekg(1 - static_cast<long long>(sizeof(stor_packet_hdr_t)), std::ios_base::cur);
    }
    ifs_.close();
}

std::vector<long> Decoder::positionsOfCMAPHeaders(const std::string &fileName)
{
    std::vector<long> pos{};
    ifs_.open(fileName, std::ios::in | std::ios::binary);
    if (!ifs_.is_open())
    {
        std::clog << "Can't open file" << std::endl;
        return pos;
    }

    stor_packet_hdr_t hdr;
    adcm_cmap_t cmap;
    adcm_counters_t counters;

    auto c{false};
    long long int currentPosition{0};
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

std::vector<long> Decoder::positionsOfNDHeaders(const std::string &fileName)
{
    std::vector<long> pos{};
    ifs_.open(fileName, std::ios::in | std::ios::binary);
    if (!ifs_.is_open())
    {
        std::clog << "Can't open file" << std::endl;
        return pos;
    }

    stor_packet_hdr_t hdr;
    stor_nd_t nd;
    adcm_counters_t counters;

    long long int currentPosition{0};
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

void Decoder::process(const std::string &fileName,
                      const long long &offset,
                      const std::pair<long long, long long> &period)
{
    ifs_.open(fileName, std::ios::in | std::ios::binary);
    if (!ifs_.is_open())
    {
        std::clog << "Can't open file" << std::endl;
        return;
    }

    counters_.rawhits.resize(pre_.numberOfChannelsAlpha());

    events_.clear();

    stor_packet_hdr_t hdr;
    stor_ev_hdr_t ev;
    adcm_cmap_t cmap;
    adcm_counters_t counters;

    stor_nd_t nd;


    auto c{false};

    auto isIntegerOverflow = [](long long int currentTs, long long int prevTs, long long int limit = 3'000'000'000){
        return std::abs(currentTs - prevTs) > limit;
    };

    std::vector<dec_ev_t> spillEvents;

    ifs_.seekg(offset, std::ios_base::beg);

    auto isSelectedSpill{true};

    while (ifs_ && isSelectedSpill)
    {
        ifs_ >> hdr;

        if (hdr.id == STOR_ID_ND && hdr.size > sizeof(stor_packet_hdr_t))
        {
            ifs_ >> nd;
            continue;
        }
        if (hdr.id == STOR_ID_CMAP && hdr.size > sizeof(stor_packet_hdr_t))
        {
            ifs_ >> cmap;
            c = pre_.isCorrect(cmap.map);

            spillEvents.clear();
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
                spillEvents.push_back(event);
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
                continue;
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
                        counters_.rawhits[number] += counters.rawhits[i];
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }

            counters_.time += counters.time;
            if (spillEvents.size())
            {

                for (size_t i{1}; i < spillEvents.size(); ++i)
                {
                    if (isIntegerOverflow(spillEvents[i].ts, spillEvents[0].ts) && spillEvents.size()) {
                        spillEvents[i].ts += UINT32_MAX;
                    }
                }
                auto tsOffset{spillEvents[0].ts};
                for (size_t i{0}; i < spillEvents.size(); ++i)
                {
                    spillEvents[i].ts -= tsOffset;
                }
                for (size_t i{0}; i < spillEvents.size(); ++i)
                {
                    auto dateInNanoSec{nd.time + spillEvents[i].ts * 10 - spillEvents[spillEvents.size() - 1].ts * 10};
                    spillEvents[i].ts = dateInNanoSec;
                }
                for (const auto &item : spillEvents)
                {
                    if (period.first <= item.ts && item.ts <= period.second)
                    {
                        events_.push_back(item);
                    }
                }
            }
            isSelectedSpill = false;
            continue;
        }
        ifs_.seekg(1 - static_cast<long long>(sizeof(stor_packet_hdr_t)), std::ios_base::cur);
    }
    ifs_.close();
}
