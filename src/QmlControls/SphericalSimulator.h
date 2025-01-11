
#include <QGeoCoordinate>

class iSphericalSimulator
{
public:

    virtual void setStartPos(QGeoCoordinate) = 0;
    virtual void setEndPos(QGeoCoordinate) = 0;
    virtual QGeoCoordinate getStartPos() const = 0;
    virtual QGeoCoordinate getEndPos() const = 0;
    virtual QGeoCoordinate getCurrentPos(int i) const = 0;
    virtual int getPosCount() const = 0;
    virtual void generatePath(int size) = 0;
    virtual bool isStartSet() const = 0;
    virtual bool isEndSet() const = 0;
};

iSphericalSimulator* getSimulator();
