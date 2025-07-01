#include "filloptions.h"

FillOptions::FillOptions(Value value) : _value{value}, _range{FillRange()}
{

}

FillOptions::FillOptions(Value value, FillRange range) : _value{value}, _range{range}
{

}

FillOptions::Value FillOptions::value() const
{
    return _value;
}

const FillRange &FillOptions::range() const
{
    return _range;
}
