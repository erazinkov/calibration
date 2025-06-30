#ifndef FILLOPTIONS_H
#define FILLOPTIONS_H


class FillOptions
{
   enum class Type {
        ENERGY,
        TIME,
    };
public:
    FillOptions(Type type);
    FillOptions(Type type, double min, double max);
private:

    Type _type;
    double _min;
    double _max;
    bool _useRange;
};

#endif // FILLOPTIONS_H
