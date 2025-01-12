#include <QList>
#include <QGeoPositionInfo>
#include <cmath>

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
        generateSinusoid(size, 20, 4);
        //generateLine(size);
    }

private:

    void generateSinusoid(int numPoints, double amplitude, double frequency)
    {
        double totalDistance = _startPos.distanceTo(_endPos);
        double azimuth = _startPos.azimuthTo(_endPos);

        double perpendicularAzimuth = azimuth + 90.0;
        double stepDistance = totalDistance / (numPoints - 1);

        for (int i = 0; i < numPoints; ++i)
        {
            double distance = i * stepDistance;

            double offset = amplitude * std::sin(2.0 * M_PI * frequency * (distance / totalDistance));
            QGeoCoordinate basePoint = _startPos.atDistanceAndAzimuth(distance, azimuth);

            QGeoCoordinate offsetPoint = basePoint.atDistanceAndAzimuth(std::abs(offset),
                                offset >= 0 ? perpendicularAzimuth : perpendicularAzimuth + 180.0);

            _path.append(offsetPoint);
        }
    }

    void generateLine(int numPoints)
    {
        double totalDistance = _startPos.distanceTo(_endPos);
        double azimuth = _startPos.azimuthTo(_endPos);
        double stepDistance = totalDistance / (numPoints - 1);

        for (int i = 0; i < numPoints; ++i) {
            double distance = i * stepDistance;
            QGeoCoordinate point = _startPos.atDistanceAndAzimuth(distance, azimuth);

            _path.append(point);
        }
    }

    QGeoCoordinate _startPos;
    QGeoCoordinate _endPos;

    QList<QGeoCoordinate> _path;
};


iSphericalSimulator* getSimulator()
{
    static SphericalSimulatorImpl impl;
    return &impl;
}
