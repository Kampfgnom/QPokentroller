#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QElapsedTimer>
#include <QSettings>

class MapController;
class XcodeController;

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

private slots:
    void on_pushButtonGetCurrentLocation_clicked();
    void on_pushButtonOpenGpx_clicked();
    void on_horizontalSliderSpeed_valueChanged(int value);

    void on_pushButtonCenter_clicked();

private:
    Ui::MainWindow *ui;
    MapController *m_mapController{nullptr};
    XcodeController *m_xcodeController{nullptr};
    QElapsedTimer m_elapsedTimer;
    qint64 m_updateInterval{500}; // ms
    QSettings m_settings;

    void timerEvent(QTimerEvent *e) override;
};

#endif // MAINWINDOW_H
