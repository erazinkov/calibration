#include "filloptions.h"

FillOptions::FillOptions(Value value) : _value{value}, _range{Range()}, _useRange{false}
{

}

FillOptions::FillOptions(Value value, Range range) : _value{value}, _range{range}, _useRange{true}
{

}

FillOptions::Value FillOptions::value() const
{
    return _value;
}

bool FillOptions::useRange() const
{
    return _useRange;
}

