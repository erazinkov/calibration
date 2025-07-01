#ifndef FILLRANGE_H
#define FILLRANGE_H

class FillRange
{
public:
    enum class Type {
        IN,
        OUT,
        IGNORE,
    };
    FillRange();
    FillRange(Type type, double min, double max, double offset);
    Type type() const;

    double min() const;

    double max() const;

    double offset() const;

private:
    Type _type;
    double _min;
    double _max;
    double _offset;
};

#endif // FILLRANGE_H
