#ifndef __BUSINESS_H__
#define __BUSINESS_H__

#include <QString>

class Business
{
    public:

        Business():
            ReviewCount(0),
            Rating(0.0),
            Latitude(0.0),
            Longitude(0.0),
            Distance(0.0),
            Name(""),
            ImageUrl(""),
            Phone(""),
            Address(""),
            City(""),
            State(""),
            ZipCode(""),
            Country("") {}

        unsigned ReviewCount;
        double Rating;
        double Latitude;
        double Longitude;
        double Distance;
        QString Name;
        QString ImageUrl;
        QString Phone;
        QString Address;
        QString City;
        QString State;
        QString ZipCode;
        QString Country;
};

#endif // __BUSINESS_H__
