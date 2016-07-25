#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <QElapsedTimer>
#include <QGeoCoordinate>
#include <QGeoRoute>
#include <QQmlListProperty>
#include <QtCore/QObject>

class QGeoRoutingManager;
class QGeoPositionInfoSource;

class MapController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate currentCoordinate READ currentCoordinate WRITE setCurrentCoordinate NOTIFY currentCoordinateChanged)
    Q_PROPERTY(QVariantList path READ path)
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed NOTIFY speedChanged)

public:
    explicit MapController(QObject *parent = 0);

    QGeoCoordinate currentCoordinate() const;
    qreal speed() const;
    QVariantList path() const;

public slots:
    void setSpeed(qreal speed);
    void setCurrentCoordinate(const QGeoCoordinate &currentCoordinate);
    void appendCalculatedRouteToPoint(QGeoCoordinate coordinate);
    void calculateRouteToPoint(QGeoCoordinate coordinate);

    void getCurrentLocation();
    void move();

signals:
    void currentCoordinateChanged(QGeoCoordinate currentCoordinate);
    void speedChanged(qreal speed);
    void centerMapOnCurrentCoordinate();
    void pathChanged();

private:
    QGeoCoordinate m_currentCoordinate;
    qreal m_azimuth{0.0}; // aka orientation
    qreal m_speed{4};     // roughly 10 miles/s in m/s
    QGeoPositionInfoSource *m_locationSource{nullptr};
    QGeoRoutingManager *m_routeManager{nullptr};
    QElapsedTimer m_timer;
    QList<QGeoCoordinate> m_restPath;
};

#endif // MAPCONTROLLER_H
