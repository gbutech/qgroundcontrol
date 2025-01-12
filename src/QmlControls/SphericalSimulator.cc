#include <QList>

#include "SphericalSimulator.h"

class SphericalSimulatorImpl : public iSphericalSimulator
{
public:

    void setStartPos(QGeoCoordinate coord)
    {
        _startPos = coord;
    }

    bool isStartSet() const
    {
        return _startPos.isValid();
    }

    bool isEndSet() const
    {
        return _endPos.isValid();
    }

    void setEndPos(QGeoCoordinate coord)
    {
        _endPos = coord;
    }

    QGeoCoordinate getStartPos() const
    {
        return _startPos;
    }

    QGeoCoordinate getEndPos() const
    {
        return _endPos;
    }

    QGeoCoordinate getCurrentPos(int i) const
    {
        assert(i >=0 && i < _path.size());
        return _path[i];
    }

    int getPosCount() const
    {
        return _path.size();
    }

    void generatePath(int size)
    {
        QGeoCoordinate pos = _startPos;
        double delta = 0.0001;

        for (int i = 0; i < size; ++i)
        {
            pos.setLatitude(pos.latitude() + delta);
            pos.setLongitude(pos.longitude() + delta);

            _path.push_back(pos);
        }
    }

private:

    QGeoCoordinate _startPos;
    QGeoCoordinate _endPos;

    QList<QGeoCoordinate> _path;
};


iSphericalSimulator* getSimulator()
{
    static SphericalSimulatorImpl impl;
    return &impl;
}
