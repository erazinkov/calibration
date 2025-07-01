#include "fillrange.h"

FillRange::FillRange() : _type{Type::IGNORE}, _min{0.0}, _max{0.0}, _offset{0.0}
{

}

FillRange::FillRange(Type type, double min, double max, double offset) : _type{type}, _min{min}, _max{max}, _offset{offset}
{

}

FillRange::Type FillRange::type() const
{
    return _type;
}

double FillRange::min() const
{
    return _min;
}

double FillRange::max() const
{
    return _max;
}

double FillRange::offset() const
{
    return _offset;
}
