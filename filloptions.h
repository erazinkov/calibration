#ifndef FILLOPTIONS_H
#define FILLOPTIONS_H


class FillOptions
{
public:
    enum class Value {
        ENERGY,
        TIME,
    };
    struct Range {
        enum class Type {
            IN,
            OUT,
        };
        double min;
        double max;
        Type type;
    };
    FillOptions(Value value);
    FillOptions(Value value, Range range);

    Value value() const;

    bool useRange() const;

private:
    Value _value;
    Range _range;
    bool _useRange;
};


#endif // FILLOPTIONS_H
