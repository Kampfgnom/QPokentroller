#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>

Q_LOGGING_CATEGORY(mainWindow, "MainWindow", QtDebugMsg)

namespace
{
static const QLatin1Literal GpxDocumentTemplate("<gpx version=\"1.1\"><wpt lat=\"%1\" lon=\"%2\"><name>CurrentLocation</name></wpt></gpx>");
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    startTimer(m_updateInterval);
    m_locationSource = QGeoPositionInfoSource::createDefaultSource(this);
    connect(m_locationSource, &QGeoPositionInfoSource::positionUpdated, [this](const QGeoPositionInfo &update) {
        m_locationSource->stopUpdates();
        setCurrentCoordinate(update.coordinate());
    });

    ui->quickWidgetMap->rootContext()->setContextProperty("controller", this);
    ui->quickWidgetMap->setSource(QUrl("qrc:quickwidgets/Map.qml"));

    bool ok = false;
    qreal latitude = m_settings.value("lastKnownLocation/latitude", 0.0).toDouble(&ok);
    Q_ASSERT(ok);
    qreal longitude = m_settings.value("lastKnownLocation/longitude", 0.0).toDouble(&ok);
    Q_ASSERT(ok);
    setCurrentCoordinate(QGeoCoordinate(latitude, longitude));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QGeoCoordinate MainWindow::currentCoordinate() const
{
    return m_currentCoordinate;
}

void MainWindow::setCurrentCoordinate(const QGeoCoordinate &currentCoordinate)
{
    if (m_currentCoordinate == currentCoordinate) {
        return;
    }

    qDebug(mainWindow) << Q_FUNC_INFO << currentCoordinate;
    m_currentCoordinate = currentCoordinate;
    ui->doubleSpinBoxLatitude->setValue(m_currentCoordinate.latitude());
    ui->doubleSpinBoxLongitude->setValue(m_currentCoordinate.longitude());
    m_settings.setValue("lastKnownLocation/latitude", m_currentCoordinate.latitude());
    m_settings.setValue("lastKnownLocation/longitude", m_currentCoordinate.longitude());

    if (ui->checkBoxActive->isChecked()) {
        writeGpxFile();
        tellXcodeToChangeLocation();
    }

    emit currentCoordinateChanged(m_currentCoordinate);
}

void MainWindow::keyReleaseEvent(QKeyEvent *keyEvent)
{
    m_keysDown.remove(keyEvent->key());
    QMainWindow::keyReleaseEvent(keyEvent);
}

void MainWindow::keyPressEvent(QKeyEvent *keyEvent)
{
    m_keysDown.insert(keyEvent->key());
    QMainWindow::keyPressEvent(keyEvent);
}

void MainWindow::timerEvent(QTimerEvent *timerEvent)
{
    for (int key : m_keysDown) {
        switch (key) {
        case Qt::Key_Left:
            turn(TurnLeft);
            break;
        case Qt::Key_Right:
            turn(TurnRight);
            break;
        }
    }

    bool moved = false;
    for (int key : m_keysDown) {
        switch (key) {
        case Qt::Key_Up:
            moved = true;
            move(MoveForward);
            break;
        case Qt::Key_Down:
            moved = true;
            move(MoveBackward);
            break;
        }
    }

    if (!moved && ui->checkBoxKeepGoing->isChecked()) {
        if (ui->checkBoxBackwards->isChecked()) {
            move(MoveBackward);
        } else {
            move(MoveForward);
        }
    }

    QMainWindow::timerEvent(timerEvent);
}

void MainWindow::move(MovementDirection direction)
{
    qDebug(mainWindow) << Q_FUNC_INFO;
    qreal distance = m_speed * m_updateInterval / 1000.0;

    switch (direction) {
    case MainWindow::MoveForward:
        break;
    case MainWindow::MoveBackward:
        distance = -distance;
        break;
    }

    qDebug(mainWindow) << "distance" << distance;

    QGeoCoordinate newCoordinate = m_currentCoordinate.atDistanceAndAzimuth(distance, m_azimuth);
    qDebug(mainWindow) << "newCoordinate" << newCoordinate;

    setCurrentCoordinate(newCoordinate);
}

void MainWindow::turn(TurningDirection direction)
{
    qDebug(mainWindow) << Q_FUNC_INFO;
    qreal deltaAzimuth = m_turningSpeed * m_updateInterval / 1000.0;
    switch (direction) {
    case MainWindow::TurnLeft:
        deltaAzimuth = -deltaAzimuth;
        break;
    case MainWindow::TurnRight:
        break;
    }
    qDebug(mainWindow) << "deltaAzimuth" << deltaAzimuth;
    m_azimuth += deltaAzimuth;

    // walk a tiny distance, to visualize the movement
    QGeoCoordinate newCoordinate = m_currentCoordinate.atDistanceAndAzimuth(1.0, m_azimuth);
    qDebug(mainWindow) << "newCoordinate" << newCoordinate;

    setCurrentCoordinate(newCoordinate);
}

void MainWindow::writeGpxFile()
{
    qDebug(mainWindow) << Q_FUNC_INFO;

    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).absoluteFilePath("pokemonLocation.gpx"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(mainWindow) << file.errorString();
        return;
    }

    QByteArray document = QString(GpxDocumentTemplate).arg(m_currentCoordinate.latitude(), 0, 'f', 14).arg(m_currentCoordinate.longitude(), 0, 'f', 14).toUtf8();
    file.write(document);
    file.close();
}

void MainWindow::runAppleScript(const QString &appleScript)
{
    static const QString osascript = "/usr/bin/osascript";
    QStringList processArguments{appleScript};
    QProcess::execute(osascript, processArguments);
}

void MainWindow::tellXcodeToChangeLocation()
{
    qDebug(mainWindow) << Q_FUNC_INFO;
    QDir dir(qApp->applicationDirPath());
    dir.cdUp();
    dir.cd("Resources");
    QString script = dir.absoluteFilePath("change-xcode-location.applescript");
    qDebug(mainWindow) << "script" << script;

    QProcess::execute(script);
}

QGeoCoordinate MainWindow::destinationCoorinate() const
{
    return m_destinationCoorinate;
}

void MainWindow::setDestinationCoorinate(QGeoCoordinate destinationCoorinate)
{
    if (m_destinationCoorinate == destinationCoorinate)
        return;

    qDebug(mainWindow) << Q_FUNC_INFO << destinationCoorinate;
    m_destinationCoorinate = destinationCoorinate;
    m_azimuth = m_currentCoordinate.azimuthTo(m_destinationCoorinate);
    emit destinationCoorinateChanged(destinationCoorinate);
}

void MainWindow::on_pushButtonGetCurrentLocation_clicked()
{
    qDebug(mainWindow) << Q_FUNC_INFO;

    m_locationSource->startUpdates();
}

void MainWindow::on_horizontalSliderSpeed_sliderMoved(int position)
{
    m_speed = static_cast<double>(position) / 10.0;
    ui->labelSpeed->setText(tr("%1 m/s").arg(m_speed));
}

void MainWindow::on_pushButtonOpenGpx_clicked()
{
    qDebug(mainWindow) << Q_FUNC_INFO;
    QString fileName = QFileDialog::getOpenFileName(this);
    qDebug(mainWindow) << "fileName" << fileName;
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(mainWindow) << "file.errorString()" << file.errorString();
        return;
    }

    QString contents = QString::fromUtf8(file.readAll());
    qDebug(mainWindow) << "contents" << contents;
    QRegularExpression regexpLat("lat=\"(\\d+\\.\\d+)\"");
    QRegularExpressionMatch matchLat = regexpLat.match(contents);
    qDebug(mainWindow) << "matchLat" << matchLat;
    if (!matchLat.isValid())
        return;

    bool ok = false;
    qreal lat = matchLat.captured(1).toDouble(&ok);
    qDebug(mainWindow) << "lat" << lat;
    if (!ok) {
        return;
    }

    QRegularExpression regexpLon("lon=\"(\\d+\\.\\d+)\"");
    QRegularExpressionMatch matchLon = regexpLon.match(contents);
    if (!matchLon.isValid())
        return;

    ok = false;
    qreal lon = matchLon.captured(1).toDouble(&ok);
    qDebug(mainWindow) << "lon" << lon;
    if (!ok) {
        return;
    }

    setCurrentCoordinate({lat, lon});
}
