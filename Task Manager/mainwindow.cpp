#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sys/statvfs.h>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupChart();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateSystemInfo()));
    timer->start(1000);

    layout = new QVBoxLayout;
    diskProgressBar = new QProgressBar(this);
    diskNameLabel = new QLabel(this);
    totalSpaceLabel = new QLabel(this);
    usedSpaceLabel = new QLabel(this);
    freeSpaceLabel = new QLabel(this);
    dateTimeLabel = new QLabel(this);

    QWidget *diskTab = new QWidget;
    QVBoxLayout *diskLayout = new QVBoxLayout;
    diskLayout->addWidget(diskProgressBar);
    diskLayout->addWidget(diskNameLabel);
    diskLayout->addWidget(totalSpaceLabel);
    diskLayout->addWidget(usedSpaceLabel);
    diskLayout->addWidget(freeSpaceLabel);
    diskLayout->addWidget(dateTimeLabel);
    diskTab->setLayout(diskLayout);
    ui->tabWidget->addTab(diskTab, "Disk");

    diskProgressBar->setRange(0, 100);

    updateDiskUsage();
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateDateTime);
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete layout;
}

void MainWindow::setupChart()
{
    ramSeries = new QLineSeries();
    cpuSeries = new QLineSeries();

    QPen ramPen(Qt::blue);
    ramPen.setWidth(2);

    ramSeries->setPen(ramPen);

    QPen cpuPen(Qt::green);
    cpuPen.setWidth(2);

    cpuSeries->setPen(cpuPen);

    QChart *ramChart = new QChart();
    ramChart->addSeries(ramSeries);
    ramChart->setTitle("RAM Usage");
    ramChart->createDefaultAxes();

    QChart *cpuChart = new QChart();
    cpuChart->addSeries(cpuSeries);
    cpuChart->setTitle("CPU Usage");
    cpuChart->createDefaultAxes();

    ramChartView = new QChartView(ramChart);
    ramChartView->setRenderHint(QPainter::Antialiasing);

    cpuChartView = new QChartView(cpuChart);
    cpuChartView->setRenderHint(QPainter::Antialiasing);

    ramDateTimeLabel = new QLabel(this);
    cpuDateTimeLabel = new QLabel(this);

    QWidget *ramTab = new QWidget;
    QVBoxLayout *ramLayout = new QVBoxLayout;
    ramLayout->addWidget(ramChartView);
    ramLayout->addWidget(new QLabel("Date/Time:"));
    ramLayout->addWidget(ramDateTimeLabel);
    ramLayout->addWidget(new QLabel("Used RAM:"));
    ramLayout->addWidget(ui->labelRAM);
    ramTab->setLayout(ramLayout);
    ui->tabWidget->addTab(ramTab, "RAM");

    QWidget *cpuTab = new QWidget;
    QVBoxLayout *cpuLayout = new QVBoxLayout;
    cpuLayout->addWidget(cpuChartView);
    cpuLayout->addWidget(new QLabel("Date/Time:"));
    cpuLayout->addWidget(cpuDateTimeLabel);
    cpuLayout->addWidget(new QLabel("Used CPU:"));
    cpuLayout->addWidget(ui->labelCPU);
    cpuTab->setLayout(cpuLayout);
    ui->tabWidget->addTab(cpuTab, "CPU");

    ui->tabWidget->removeTab(0);
    ui->tabWidget->removeTab(0);
}

void MainWindow::updateChart(QLineSeries *series, QChartView *chartView, double usage)
{
    series->append(series->count(), usage);

    if (series->count() > 1000) {
        series->remove(0);
    }

    QtCharts::QValueAxis *xAxis = qobject_cast
        <QtCharts::QValueAxis *>(chartView->chart()->axisX(series));
    if (xAxis) {
        xAxis->setRange(0, series->count());
        xAxis->setTickCount(2);
        xAxis->setLabelFormat("%d");
    }

    QtCharts::QValueAxis *yAxis = qobject_cast
        <QtCharts::QValueAxis *>(chartView->chart()->axisY(series));
    if (yAxis) {
        yAxis->setRange(0, 100);
        yAxis->setTickCount(5);
        yAxis->setLabelFormat("%.0f");
    }
}

void MainWindow::updateSystemInfo()
{
    double UsedRam = getUsedRam();
    double UsedCpu = getUsedCpu();

    updateChart(ramSeries, ramChartView, UsedRam);
    updateChart(cpuSeries, cpuChartView, UsedCpu);

    ui->labelRAM->setText(QString::number(UsedRam, 'f', 2));
    ui->labelCPU->setText(QString::number(UsedCpu, 'f', 2));

    QDateTime currentDateTime = QDateTime::currentDateTime();
    ramDateTimeLabel->setText(currentDateTime.toString(Qt::ISODate));
    cpuDateTimeLabel->setText(currentDateTime.toString(Qt::ISODate));
}

double MainWindow::getUsedRam()
{
    std::ifstream file("/proc/meminfo");
    std::string line;
    long totalMem = 0, freeMem = 0, buffers = 0, cached = 0;

    while (getline(file, line))
    {
        std::istringstream ss(line);
        std::string label;
        long value;

        ss >> label >> value;

        if (label == "MemTotal:")
            totalMem = value;
        else if (label == "MemFree:")
            freeMem = value;
        else if (label == "Buffers:")
            buffers = value;
        else if (label == "Cached:")
            cached = value;
    }

    long usedMem = totalMem - freeMem - buffers - cached;
    double ramUsage = 100.0 * static_cast<double>(usedMem) / totalMem;

    return ramUsage;
}

double MainWindow::getUsedCpu()
{
    std::ifstream file("/proc/stat");
    std::string line;
    getline(file, line);
    std::istringstream ss(line);
    std::string cpuLabel;
    ss >> cpuLabel;

    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    long totalCpuTime = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
    long idleTime = idle + iowait;

    double cpuUsage = 100.0 * (1.0 - static_cast<double>(idleTime) / totalCpuTime);

    return cpuUsage;
}

void MainWindow::updateDiskUsage()
{
    struct statvfs buf;
    if (statvfs("/", &buf) == 0) {
        qint64 totalSpace = buf.f_frsize * buf.f_blocks;
        qint64 freeSpace = buf.f_frsize * buf.f_bavail;
        qint64 usedSpace = totalSpace - freeSpace;

        int usagePercentage = 0;
        if (totalSpace > 0) {
            usagePercentage = 100 - ((freeSpace * 100) / totalSpace);
        }
        diskProgressBar->setValue(usagePercentage);

        totalSpaceLabel->setText("Total Space: " + QString::number(totalSpace / (1024 * 1024)) + " MB");
        usedSpaceLabel->setText("Used Space: " + QString::number(usedSpace / (1024 * 1024)) + " MB");
        freeSpaceLabel->setText("Free Space: " + QString::number(freeSpace / (1024 * 1024)) + " MB");
    }
}

void MainWindow::updateDateTime()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    dateTimeLabel->setText("Date and Time: " + currentDateTime.toString(Qt::ISODate));
}
