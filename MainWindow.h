#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtLocation>
#include <QtWidgets/QMainWindow>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QGeoCoordinate currentCoordinate() const;
    void setCurrentCoordinate(const QGeoCoordinate &currentCoordinate);

    void runAppleScript(const QString &appleScript);
    void tellXcodeToChangeLocation();

private slots:
    void on_pushButtonGetCurrentLocation_clicked();

    void on_horizontalSliderSpeed_sliderMoved(int position);

private:
    Ui::MainWindow *ui;
    QGeoCoordinate m_currentCoordinate{51.5040137074481, 7.47502049276901};
    qreal m_azimuth{0.0};       // aka orientation
    qreal m_turningSpeed{22.5}; // azimuth degrees per second
    qreal m_speed{4};        // roughly 10 miles/s in m/s
    QElapsedTimer m_elapsedTimer;
    qint64 m_updateInterval{500}; // ms
    QSet<int> m_keysDown;
    QGeoPositionInfoSource *m_locationSource{nullptr};
    QSettings m_settings;

    void keyReleaseEvent(QKeyEvent *keyEvent) override;
    void keyPressEvent(QKeyEvent *keyEvent) override;
    void timerEvent(QTimerEvent *timerEvent) override;

    enum MovementDirection { MoveForward, MoveBackward };
    void move(MovementDirection direction);
    enum TurningDirection { TurnLeft, TurnRight };
    void turn(TurningDirection direction);

    void writeGpxFile();
};

#endif // MAINWINDOW_H
