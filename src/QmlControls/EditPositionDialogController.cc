/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "EditPositionDialogController.h"
#include "QGCGeo.h"
#include "MultiVehicleManager.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"
#include "QGCApplication.h"
#include "QQuickWindow"
#include <thread>

QGC_LOGGING_CATEGORY(EditPositionDialogControllerLog, "qgc.qmlcontrols.editpositiondialogcontroller")

QMap<QString, FactMetaData*> EditPositionDialogController::_metaDataMap;

EditPositionDialogController::EditPositionDialogController(QObject *parent)
    : QObject(parent)
    , _latitudeFact(new Fact(0, _latitudeFactName, FactMetaData::valueTypeDouble, this))
    , _longitudeFact(new Fact(0, _longitudeFactName, FactMetaData::valueTypeDouble, this))
    , _zoneFact(new Fact(0, _zoneFactName, FactMetaData::valueTypeUint8, this))
    , _hemisphereFact(new Fact(0, _hemisphereFactName, FactMetaData::valueTypeUint8, this))
    , _eastingFact(new Fact(0, _eastingFactName, FactMetaData::valueTypeDouble, this))
    , _northingFact(new Fact(0, _northingFactName, FactMetaData::valueTypeDouble, this))
    , _mgrsFact(new Fact(0, _mgrsFactName, FactMetaData::valueTypeString, this))
{
    // qCDebug(EditPositionDialogControllerLog) << Q_FUNC_INFO << this;

    if (_metaDataMap.isEmpty()) {
        _metaDataMap = FactMetaData::createMapFromJsonFile(QStringLiteral(":/json/EditPositionDialog.FactMetaData.json"), nullptr /* QObject parent */);
    }

    _latitudeFact->setMetaData(_metaDataMap[_latitudeFactName]);
    _longitudeFact->setMetaData(_metaDataMap[_longitudeFactName]);
    _zoneFact->setMetaData(_metaDataMap[_zoneFactName]);
    _hemisphereFact->setMetaData(_metaDataMap[_hemisphereFactName]);
    _eastingFact->setMetaData(_metaDataMap[_eastingFactName]);
    _northingFact->setMetaData(_metaDataMap[_northingFactName]);
    _mgrsFact->setMetaData(_metaDataMap[_mgrsFactName]);
}

EditPositionDialogController::~EditPositionDialogController()
{
    // qCDebug(EditPositionDialogControllerLog) << Q_FUNC_INFO << this;
}

class PositionSimulator
{
public:
    PositionSimulator(EditPositionDialogController*) {}

    void setStartPosition(double latitude, double longitude)
    {
        _startPos.setX((longitude - minLon) / (maxLon - minLon));
        _startPos.setY((maxLat - latitude) / (maxLat - minLat));
    }

    void setEndPosition(double latitude, double longitude)
    {
        _endPos.setX((longitude - minLon) / (maxLon - minLon));
        _endPos.setY((maxLat - latitude) / (maxLat - minLat));
    }

    QPointF getStartPos()
    {
        return _startPos;
    }

    QPointF getEndPos()
    {
        return _endPos;
    }

    QPointF getFirstPathPos(int i)
    {
        assert(i >=0 && i < _firstPath.size());
        return _firstPath[i];
    }

    int getFirstPathSize() const
    {
        return _firstPath.size();
    }

    void generateFirstPath(int size)
    {
        QPointF pos = _startPos;
        _firstPath.push_back(pos);
        double delta = 0.01;
        for (int i = 0; i < size; ++i)
        {
            pos += QPointF(delta, delta);
            _firstPath.push_back(pos);
        }
    }

private:
    const int minLat = -90;
    const int maxLat = 90;
    const int minLon = -180;
    const int maxLon = 180;

    QPointF _startPos;
    QPointF _endPos;

    QList<QPointF> _firstPath;
    QList<QPointF> _secondPath;
};

void EditPositionDialogController::setCoordinate(QGeoCoordinate coordinate)
{
    //Mamikon
    auto ellipseItem = qgcApp()->getEllipseItem();
    ellipseItem->setFlag(QQuickItem::ItemHasContents, true);
    ellipseItem->setProperty("color", "blue");
    ellipseItem->setProperty("shouldPaint", true);

    //double latitude = 40.1553279;
    //double longitude = 44.5094510;

    PositionSimulator* sim = new PositionSimulator(this);
    sim->setStartPosition(coordinate.latitude(), coordinate.longitude());
    sim->generateFirstPath(10);
    qDebug() << "POS lat long " << coordinate.latitude() << " " << coordinate.longitude();

    //ellipseItem->addPosition(QPointF(sim->getStartPos().x(), sim->getStartPos().y()));
    //QSize newSize = window->size() + QSize(1, 1);
    //window->resize(newSize);

    QObject *canvasRef = ellipseItem->property("ellipseCanvasRef").value<QObject *>();
    if (canvasRef) {
        QQuickItem *canvasItem = qobject_cast<QQuickItem *>(canvasRef);
        if (canvasItem) {
            std::thread drawThread([canvasItem, ellipseItem, sim]() {
                for (int i = 0; i < sim->getFirstPathSize(); ++i) {

                    ellipseItem->addPosition(sim->getFirstPathPos(i));
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    QMetaObject::invokeMethod(canvasItem, "requestPaint");

                    // Update GUI safely on the main thread
                    /*QMetaObject::invokeMethod(canvasItem, [canvasItem]() {
                        QMetaObject::invokeMethod(canvasItem, "requestPaint");
                    });*/
                }
            });
            //if (drawThread.joinable())
              //  drawThread.join();

            drawThread.detach();
            /*QObject::connect(qApp, &QCoreApplication::aboutToQuit, [&]() {
                if (drawThread.joinable())
                    drawThread.join();
            });*/
        }
        // Ensure the thread is safely stopped when the application exits


        //drawThread.detach();

        /*QTimer* timer = new QTimer(canvasItem);
        int i = 0;
        QObject::connect(timer, &QTimer::timeout, canvasItem, [&]() {

            ellipseItem->addPosition(sim.getFirstPathPos(i));
            ++i;
            if (i == 3)
                timer->stop();

                //QSize newSize = window->size() + QSize(1, 1);
                //window->resize(newSize);
            QMetaObject::invokeMethod(canvasItem, "requestPaint");
            });
        timer->start(1000);*/
    }

    /*std::thread tr([&] () {
        for (int i = 0; i < 10; ++i)
        {
            ellipseItem->addPosition(sim.getFirstPathPos(i));

            QSize newSize = window->size() + QSize(1, 1);
            window->resize(newSize);
            sleep(1);
        }
    });
    tr.join();*/

    if (coordinate != _coordinate) {
        _coordinate = coordinate;
        qDebug() << "POS lat long " << coordinate.latitude() << " " << coordinate.longitude();
        emit coordinateChanged(coordinate);
    }

}

void EditPositionDialogController::initValues()
{
    _latitudeFact->setRawValue(_coordinate.latitude());
    _longitudeFact->setRawValue(_coordinate.longitude());

    double easting, northing;
    const int zone = QGCGeo::convertGeoToUTM(_coordinate, easting, northing);
    if ((zone >= 1) && (zone <= 60)) {
        _zoneFact->setRawValue(zone);
        _hemisphereFact->setRawValue(_coordinate.latitude() < 0);
        _eastingFact->setRawValue(easting);
        _northingFact->setRawValue(northing);
    }

    const QString mgrs = QGCGeo::convertGeoToMGRS(_coordinate);
    if (!mgrs.isEmpty()) {
        _mgrsFact->setRawValue(mgrs);
    }
}

void EditPositionDialogController::setFromGeo()
{
    _coordinate.setLatitude(_latitudeFact->rawValue().toDouble());
    _coordinate.setLongitude(_longitudeFact->rawValue().toDouble());
    emit coordinateChanged(_coordinate);
}

void EditPositionDialogController::setFromUTM()
{
    qCDebug(EditPositionDialogControllerLog) << _eastingFact->rawValue().toDouble() << _northingFact->rawValue().toDouble() << _zoneFact->rawValue().toInt() << (_hemisphereFact->rawValue().toInt() == 1);
    if (QGCGeo::convertUTMToGeo(_eastingFact->rawValue().toDouble(), _northingFact->rawValue().toDouble(), _zoneFact->rawValue().toInt(), _hemisphereFact->rawValue().toInt() == 1, _coordinate)) {
        qCDebug(EditPositionDialogControllerLog) << _eastingFact->rawValue().toDouble() << _northingFact->rawValue().toDouble() << _zoneFact->rawValue().toInt() << (_hemisphereFact->rawValue().toInt() == 1) << _coordinate;
        emit coordinateChanged(_coordinate);
    } else {
        initValues();
    }
}

void EditPositionDialogController::setFromMGRS()
{
    if (QGCGeo::convertMGRSToGeo(_mgrsFact->rawValue().toString(), _coordinate)) {
        emit coordinateChanged(_coordinate);
    } else {
        initValues();
    }
}

void EditPositionDialogController::setFromVehicle()
{
    setCoordinate(MultiVehicleManager::instance()->activeVehicle()->coordinate());
}

