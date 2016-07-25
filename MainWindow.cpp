#include "MainWindow.h"
#include "MapController.h"
#include "XcodeController.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QLoggingCategory>
#include <QQmlContext>

Q_LOGGING_CATEGORY(mainWindow, "MainWindow", QtDebugMsg)


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_mapController = new MapController(this);
    m_xcodeController = new XcodeController(this);
    m_xcodeController->setMapController(m_mapController);

    ui->quickWidgetMap->rootContext()->setContextProperty("controller", m_mapController);
    ui->quickWidgetMap->setSource(QUrl("qrc:quickwidgets/Map.qml"));

    qreal latitude = m_settings.value("lastKnownLocation/latitude", 0.0).toDouble();
    qreal longitude = m_settings.value("lastKnownLocation/longitude", 0.0).toDouble();
    m_mapController->setCurrentCoordinate(QGeoCoordinate(latitude, longitude));

    startTimer(30);

    m_mapController->getCurrentLocation();

    connect(m_mapController, &MapController::speedChanged, [this] (qreal speed){
        ui->labelSpeed->setText(QString::number(speed));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonGetCurrentLocation_clicked()
{
    qDebug(mainWindow) << Q_FUNC_INFO;
    m_mapController->getCurrentLocation();
}

void MainWindow::on_horizontalSliderSpeed_valueChanged(int value)
{
    m_mapController->setSpeed(static_cast<double>(value) / 10.0);
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

    m_mapController->setCurrentCoordinate({lat, lon});
}

void MainWindow::on_pushButtonCenter_clicked()
{
    emit m_mapController->centerMapOnCurrentCoordinate();
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    QMainWindow::timerEvent(e);
    m_mapController->move();
}
