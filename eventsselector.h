#ifndef EVENTSSELECTOR_H
#define EVENTSSELECTOR_H

#include <vector>
#include <adcm_df.h>
#include <algorithm>

template <typename T>
class EventsSelector
{
public:
    EventsSelector(std::vector<T> events) : events{events} {};
    virtual std::vector<T> selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha) = 0;
    std::vector<T> events;
};

class EventsSelector2p : public EventsSelector<dec_ev_2p_t> {
    EventsSelector2p(std::vector<dec_ev_2p_t> events) : EventsSelector{events} {};
    std::vector<dec_ev_2p_t> selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha) override;
};

class EventsSelector3p : public EventsSelector<dec_ev_3p_t> {
    EventsSelector3p(std::vector<dec_ev_3p_t> events) : EventsSelector{events} {};
    std::vector<dec_ev_3p_t> selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha) override;
};

#endif // EVENTSSELECTOR_H
