#include "MapController.h"

#include <QElapsedTimer>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QGeoRouteSegment>
#include <QGeoRoutingManager>
#include <QGeoServiceProvider>
#include <QDebug>

MapController::MapController(QObject *parent) : QObject(parent)
{
    m_locationSource = QGeoPositionInfoSource::createDefaultSource(this);
    QGeoServiceProvider *provider = new QGeoServiceProvider("osm");
    m_routeManager = provider->routingManager();

    connect(m_routeManager, &QGeoRoutingManager::finished, [this](QGeoRouteReply *reply) {
        if (reply->error() != QGeoRouteReply::NoError) {
            qWarning() << reply->errorString();
            qFatal("error");
        }

        if (reply->routes().isEmpty()) {
            qFatal("no routes");
        }

        auto path = reply->routes().first().path();
        qreal endDistance = reply->request().waypoints().last().distanceTo(path.last());
        if(endDistance > 5.0)
            path.append(reply->request().waypoints().last());

        m_restPath.append(path);
        emit pathChanged();
    });

    connect(m_locationSource, &QGeoPositionInfoSource::positionUpdated, [this](const QGeoPositionInfo &update) {
        m_locationSource->stopUpdates();
        setCurrentCoordinate(update.coordinate());
        emit centerMapOnCurrentCoordinate();
    });
}

QGeoCoordinate MapController::currentCoordinate() const
{
    return m_currentCoordinate;
}

void MapController::getCurrentLocation()
{
    m_locationSource->startUpdates();
}

void MapController::move()
{
    if (m_restPath.isEmpty()) {
        m_timer.invalidate();
        return;
    }

    if (!m_timer.isValid()) {
        m_timer.start();
        return;
    }

    qint64 elapsed = m_timer.restart();
    qreal distance = m_speed * elapsed / 1000.0;

    for (auto next = m_restPath.first(); !m_restPath.isEmpty(); next = m_restPath.takeFirst()) {
        m_azimuth = m_currentCoordinate.azimuthTo(next);
        qreal nextDistance = m_currentCoordinate.distanceTo(next);

        if (distance * 2.0 < nextDistance) {
            break;
        }
    }

    setCurrentCoordinate(m_currentCoordinate.atDistanceAndAzimuth(distance, m_azimuth));
}

void MapController::setSpeed(qreal speed)
{
    if (m_speed == speed)
        return;

    m_speed = speed;
    emit speedChanged(speed);
}

QVariantList MapController::path() const
{
    QVariantList result;
    result << QVariant::fromValue<QGeoCoordinate>(m_currentCoordinate);
    for (auto c : m_restPath) {
        result << QVariant::fromValue<QGeoCoordinate>(c);
    }
    return result;
}

qreal MapController::speed() const
{
    return m_speed;
}

void MapController::setCurrentCoordinate(const QGeoCoordinate &currentCoordinate)
{
    if (m_currentCoordinate == currentCoordinate)
        return;

    m_currentCoordinate = currentCoordinate;
    emit currentCoordinateChanged(m_currentCoordinate);
}

void MapController::appendCalculatedRouteToPoint(QGeoCoordinate coordinate)
{
    QGeoCoordinate start = m_restPath.isEmpty() ? m_currentCoordinate : m_restPath.last();
    QGeoRouteRequest request(start, coordinate);
    request.setTravelModes(QGeoRouteRequest::PedestrianTravel);

    m_routeManager->calculateRoute(request);
}

void MapController::calculateRouteToPoint(QGeoCoordinate coordinate)
{
    m_restPath.clear();
    appendCalculatedRouteToPoint(coordinate);
}
