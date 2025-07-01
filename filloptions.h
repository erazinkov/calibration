#ifndef FILLOPTIONS_H
#define FILLOPTIONS_H

#include "fillrange.h"

class FillOptions
{
public:
    enum class Value {
        CHANNEL,
        ENERGY,
        TIME,
    };
    FillOptions(Value value);
    FillOptions(Value value, FillRange range);

    Value value() const;

    const FillRange &range() const;

private:
    Value _value;
    FillRange _range;
};


#endif // FILLOPTIONS_H
