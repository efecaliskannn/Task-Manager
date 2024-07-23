// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QObject>
#include <QLabel>
#include <fstream>
#include <sstream>
#include <QtCharts/QValueAxis>
#include <QDateTime>
#include <QVBoxLayout>
#include <QProgressBar>

QT_CHARTS_USE_NAMESPACE

    namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateSystemInfo();
    void updateDateTime();
    void updateDiskUsage();
private:
    Ui::MainWindow *ui;

    QTimer *timer;
    QLabel *ramDateTimeLabel;
    QLabel *cpuDateTimeLabel;

    QLineSeries *ramSeries;
    QLineSeries *cpuSeries;

    QChartView *chartView;
    QChartView *ramChartView;
    QChartView *cpuChartView;

    QLabel *dateTimeLabel;
    QLabel *ramUsageLabel;
    QLabel *cpuUsageLabel;

    QVBoxLayout *layout;
    QProgressBar *diskProgressBar;
    QLabel *diskNameLabel;
    QLabel *totalSpaceLabel;
    QLabel *usedSpaceLabel;
    QLabel *freeSpaceLabel;

    void setupChart();
    void updateChart(QLineSeries *series, QChartView *chartView, double usage);
    double getUsedRam();
    double getUsedCpu();
};

#endif // MAINWINDOW_H
