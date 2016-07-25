#include "XcodeController.h"
#include "MapController.h"

#include <QApplication>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>

namespace
{
static const QLatin1Literal GpxDocumentTemplate("<gpx version=\"1.1\"><wpt lat=\"%1\" lon=\"%2\"><name>CurrentLocation</name></wpt></gpx>");
}

XcodeController::XcodeController(QObject *parent) : QObject(parent)
{
    startTimer(1000);
}

void XcodeController::tellXcodeToChangeLocation()
{
    if (!m_mapController)
        return;

    QGeoCoordinate current = m_mapController->currentCoordinate();
    if (current == m_previousCoordinate) {
        return;
    }

    m_previousCoordinate = current;

    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).absoluteFilePath("pokemonLocation.gpx"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(-10, 10);
    qreal lat = 0.000001 * dis(gen);
    qreal lon = 0.000001 * dis(gen);
    current.setLatitude(current.latitude() + lat);
    current.setLongitude(current.longitude() + lon);

    QByteArray document = QString(GpxDocumentTemplate).arg(current.latitude(), 0, 'f', 14).arg(current.longitude(), 0, 'f', 14).toUtf8();
    file.write(document);
    file.close();

    QDir dir(qApp->applicationDirPath());
    dir.cdUp();
    dir.cd("Resources");
    QString script = dir.absoluteFilePath("change-xcode-location.applescript");

    QProcess::execute(script);
}

MapController *XcodeController::mapController() const
{
    return m_mapController;
}

void XcodeController::setMapController(MapController *mapController)
{
    m_mapController = mapController;
}

void XcodeController::timerEvent(QTimerEvent *event)
{
    QObject::timerEvent(event);
    tellXcodeToChangeLocation();
}
