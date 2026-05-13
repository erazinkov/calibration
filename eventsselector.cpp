#include "eventsselector.h"

std::vector<dec_ev_2p_t> EventsSelector2p::selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha) {
    std::vector<dec_ev_2p_t> selectedEvents{};
    auto it{events.begin()};

    while ( (it = std::find_if(it, events.end(), [&idxGamma, &idxAlpha](dec_ev_2p_t e){
                               return e.g.index == idxGamma && e.a.index == idxAlpha;
})) != events.end() ) {
        selectedEvents.push_back(*it);
        ++it;
    }
    return selectedEvents;
}
