#include "decoder.h"

#include <iostream>
#include <bits/stdc++.h>

#include "progressbar.h"

Decoder::Decoder(const std::string &fileName, const ChannelMap &pre)
    : fileName_{fileName} , map_{pre}
{
    process();
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
    u_int64_t size{static_cast<u_int64_t>(ifs_.tellg())};
    ifs_.seekg(0);

    auto number{map_.getIdxsByType(Channel::ALPHA).size() + map_.getIdxsByType(Channel::GAMMA).size()};
    counters_.rawhits.resize(map_.map().size());

    events_2p_.clear();

    stor_packet_hdr_t hdr;
    stor_ev_hdr_t ev;
    adcm_cmap_t cmap;
    adcm_counters_t counters;

    auto c{false};
    u_int64_t currentPosition{0};
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
            c = map_.isCorrect(cmap.map);
            if (c)
            {
                currentPosition -= sizeof(stor_packet_hdr_t);
                ProgressBar<u_int64_t>::show(currentPosition, size);
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
            switch (ev.np) {
            case 2:
            {
                std::unique_ptr<stor_puls_t> g{std::make_unique<stor_puls_t>()};
                std::unique_ptr<stor_puls_t> a{std::make_unique<stor_puls_t>()};
                ifs_ >> *g >> *a;
                auto type_0{map_.map().at(g->ch).type()};
                auto type_1{map_.map().at(a->ch).type()};
                auto idxGamma{map_.getIdxByHardwareIdx(g->ch)};
                auto idxAlpha{map_.getIdxByHardwareIdx(a->ch)};
                if (idxGamma.has_value() && idxAlpha.has_value() && type_0 == Channel::GAMMA && type_1 == Channel::ALPHA) {
                    dec_ev_2p_t event;
                    event.g.index = idxGamma.value();
                    event.g.amp = g->a;
                    event.a.index = idxAlpha.value();
                    event.a.amp = a->a;
                    event.tdc = g->t - a->t;
                    event.g.time = g->t;
                    event.a.time = a->t;
                    double currentTs{static_cast<double>(ev.ts)};
                    event.ts = currentTs;
                    if (isIntegerOverflow(event.ts, prevTs) && events_2p_.size()) {
                        event.ts += UINT32_MAX;
                    }
                    prevTs = event.ts;
                    events_2p_.push_back(event);
                }
                break;
            }
//            case 3:
//            {
//                std::unique_ptr<stor_puls_t> g_0{std::make_unique<stor_puls_t>()};
//                std::unique_ptr<stor_puls_t> g_1{std::make_unique<stor_puls_t>()};
//                std::unique_ptr<stor_puls_t> a{std::make_unique<stor_puls_t>()};
//                ifs_ >> *g_0 >> *g_1 >> *a;
//                auto type_0{map_.map().at(g_0->ch).type()};
//                auto type_1{map_.map().at(g_1->ch).type()};
//                auto type_2{map_.map().at(a->ch).type()};
//                auto idxGamma_0{map_.getIdxByHardwareIdx(g_0->ch)};
//                auto idxGamma_1{map_.getIdxByHardwareIdx(g_1->ch)};
//                auto idxAlpha{map_.getIdxByHardwareIdx(a->ch)};
//                if (idxGamma_0.has_value() && idxGamma_1.has_value() && idxAlpha.has_value()
//                        && type_0 == Channel::GAMMA && type_1 == Channel::GAMMA && type_2 == Channel::ALPHA) {
//                    dec_ev_3p_t event;
//                    event.g_0.index = idxGamma_0.value();
//                    event.g_0.amp = g_0->a;
//                    event.g_1.index = idxGamma_1.value();
//                    event.g_1.amp = g_1->a;
//                    event.a.index = idxAlpha.value();
//                    event.a.amp = a->a;
//                    event.g_0.time = g_0->t;
//                    event.g_1.time = g_1->t;
//                    event.a.time = a->t;
//                    double currentTs{static_cast<double>(ev.ts)};
//                    event.ts = currentTs;
//                    if (isIntegerOverflow(event.ts, prevTs) && events_3p_.size()) {
//                        event.ts += UINT32_MAX;
//                    }
//                    prevTs = event.ts;
//                    events_3p_.push_back(event);
//                }
//                break;
//            }
            default:
            {
                hdr.size -= sizeof(stor_packet_hdr_t);
                hdr.size -= sizeof(stor_ev_hdr_t);
                ifs_.ignore(hdr.size);
            }
            }
//            std::string key{""};
//            for (auto pIdx = 0; pIdx < ev.np; pIdx++) {
//                stor_puls_t *p = new stor_puls_t();
//                ifs_ >> *p;
//                auto type{map_.map().at(p->ch).type()};
//                switch (type) {
//                    case Channel::ALPHA:
//                        key.append("a");
//                    break;
//                    case Channel::GAMMA:
//                        key.append("g");
//                    break;
//                    case Channel::UNKNOWN:
//                        key.append("u");
//                    break;
//                }
//                delete p;
//            }
//            if (key.length()) {
//                if (pulses_.count(key)) {
//                    pulses_.at(key) += 1;
//                } else {
//                    pulses_[key] = 1;
//                }
//            }

            //

            continue;

//            stor_puls_t *g = new stor_puls_t();
//            stor_puls_t *a = new stor_puls_t();
//            ifs_ >> *g >> *a;


//            auto idxGamma{map_.getIdxByHardwareIdx(g->ch)};
//            auto idxAlpha{map_.getIdxByHardwareIdx(a->ch)};
//            if (idxGamma.has_value() && idxAlpha.has_value())
//            {
//                dec_ev_t event;
//                event.g.index = idxGamma.value();
//                event.g.amp = g->a;
//                event.a.index = idxAlpha.value();
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
////            if (g->ch < _map.map().size() && a->ch < _map.map().size())
////            {
////                dec_ev_t event;
////                auto idxGamma{_map.map().at(g->ch).softwareIndex()};
////                auto idxAlpha{_map.map().at(a->ch).softwareIndex()};
////                event.g.index = idxGamma;
////                event.g.amp = g->a;
////                event.a.index = idxAlpha;
////                event.a.amp = a->a;
////                event.tdc = g->t - a->t;
////                double currentTs{static_cast<double>(ev.ts)};
////                event.ts = currentTs;
////                if (isIntegerOverflow(event.ts, prevTs) && events_.size()) {
////                    event.ts += UINT32_MAX;
////                }
////                prevTs = event.ts;
////                events_.push_back(event);
////            }
//            delete g;
//            delete a;
//            continue;
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
            for (size_t i{0}; i < map_.map().size(); ++i)
            {
                auto idx{map_.map().at(i).index()};
                auto type{map_.map().at(i).type()};
                if (type == Channel::UNKNOWN)
                {
                    continue;
                }
                counters_.rawhits.at(i) += counters.rawhits.at(i);
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

const std::map<std::string, u_int64_t> &Decoder::pulses() const
{
    return pulses_;
}

const std::vector<dec_ev_2p_t> &Decoder::events_2p() const
{
    return events_2p_;
}

const std::vector<dec_ev_3p_t> &Decoder::events_3p() const
{
    return events_3p_;
}


