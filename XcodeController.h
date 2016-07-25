#ifndef XCODECONTROLLER_H
#define XCODECONTROLLER_H

#include <QObject>
#include <QGeoCoordinate>
#include <random>

class MapController;

class XcodeController : public QObject
{
    Q_OBJECT
public:
    explicit XcodeController(QObject *parent = 0);

    void tellXcodeToChangeLocation();

    MapController *mapController() const;
    void setMapController(MapController *mapController);

private:
    std::random_device rd;

    QGeoCoordinate m_previousCoordinate;
    MapController *m_mapController{nullptr};

    void timerEvent(QTimerEvent *event) override;
};

#endif // XCODECONTROLLER_H
