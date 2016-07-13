#include "MainWindow.h"
#include "ui_MainWindow.h"

Q_LOGGING_CATEGORY(mainWindow, "MainWindow", QtDebugMsg)

namespace
{
static const QLatin1Literal GpxDocumentTemplate("<gpx version=\"1.1\"><wpt lat=\"%1\" lon=\"%2\"><name>CurrentLocation</name></wpt></gpx>");
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    startTimer(m_updateInterval);
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
    writeGpxFile();
    tellXcodeToChangeLocation();
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

    for (int key : m_keysDown) {
        switch (key) {
        case Qt::Key_Up:
            move(MoveForward);
            break;
        case Qt::Key_Down:
            move(MoveBackward);
            break;
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
