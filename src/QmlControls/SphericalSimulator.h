
#include <QGeoCoordinate>

class iSphericalSimulator
{
public:

    virtual void setStartPos(double latitude, double longitude) = 0;
    virtual void setEndPos(double latitude, double longitude) = 0;
    virtual QGeoCoordinate getStartPos() const = 0;
    virtual QGeoCoordinate getEndPos() const = 0;
    virtual QGeoCoordinate getCurrentPos(int i) const = 0;
    virtual int getPosCount() const = 0;
    virtual void generatePath(int size) = 0;
};

iSphericalSimulator* getSimulator();
