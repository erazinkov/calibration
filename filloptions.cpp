#include "filloptions.h"

FillOptions::FillOptions(Type type) : _type{type}, _min{0.0}, _max{0.0}, _useRange{false}
{

}

FillOptions::FillOptions(Type type, double min, double max) : _type{type}, _min{min}, _max{max}, _useRange{true}
{

}

