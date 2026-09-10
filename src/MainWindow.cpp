#include "MainWindow.h"

#include <QAction>
#include <QComboBox>
#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QStatusBar>
#include <QVBoxLayout>

#include "SerialPort.h"
#include "Tc66Device.h"

namespace
{
QLabel *makeValueLabel()
{
    auto *label = new QLabel(QStringLiteral("--"));
    QFont font = label->font();
    font.setFamily(QStringLiteral("Consolas"));
    font.setStyleHint(QFont::Monospace);
    font.setBold(true);
    font.setPointSize(font.pointSize() + 1);
    label->setFont(font);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setMinimumWidth(180);
    return label;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("RDTech TC66 Monitor"));

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    layout->addWidget(buildConnectionGroup());
    layout->addWidget(buildDeviceGroup());
    layout->addWidget(buildReadingsGroup());
    layout->addStretch();

    setCentralWidget(central);
    statusBar();

    auto *editMenu = menuBar()->addMenu(QStringLiteral("&Edit"));
    auto *preferencesAction = editMenu->addAction(QStringLiteral("&Preferences..."));
    preferencesAction->setShortcut(QKeySequence::Preferences);
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::openPreferences);

    m_device = new Tc66Device(this);
    connect(m_device, &Tc66Device::readingReady, this, &MainWindow::onReadingReady);
    connect(m_device, &Tc66Device::connectionChanged, this, &MainWindow::onConnectionChanged);
    connect(m_device, &Tc66Device::errorOccurred, this, &MainWindow::onError);

    loadColors();
    applyColors();
    refreshPorts();
}

MainWindow::~MainWindow() = default;

QWidget *MainWindow::buildConnectionGroup()
{
    auto *group = new QGroupBox(QStringLiteral("Connection"), this);
    auto *layout = new QHBoxLayout(group);

    layout->addWidget(new QLabel(QStringLiteral("Serial port:"), group));

    m_portCombo = new QComboBox(group);
    m_portCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(m_portCombo);

    m_refreshButton = new QPushButton(QStringLiteral("Refresh"), group);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    layout->addWidget(m_refreshButton);

    m_connectButton = new QPushButton(QStringLiteral("Connect"), group);
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::toggleConnection);
    layout->addWidget(m_connectButton);

    return group;
}

QWidget *MainWindow::buildDeviceGroup()
{
    auto *group = new QGroupBox(QStringLiteral("Device"), this);
    auto *form = new QFormLayout(group);

    m_modelValue = makeValueLabel();
    m_firmwareValue = makeValueLabel();
    m_serialValue = makeValueLabel();

    form->addRow(QStringLiteral("Model:"), m_modelValue);
    form->addRow(QStringLiteral("Firmware:"), m_firmwareValue);
    form->addRow(QStringLiteral("Serial:"), m_serialValue);

    return group;
}

QWidget *MainWindow::buildReadingsGroup()
{
    auto *group = new QGroupBox(QStringLiteral("Measurements"), this);
    auto *form = new QFormLayout(group);

    m_voltageValue = addRow(form, QStringLiteral("Voltage"));
    m_currentValue = addRow(form, QStringLiteral("Current"));
    m_powerValue = addRow(form, QStringLiteral("Power"));
    m_resistanceValue = addRow(form, QStringLiteral("Resistance"));
    m_mah0Value = addRow(form, QStringLiteral("Capacity group 0 (mAh)"));
    m_mwh0Value = addRow(form, QStringLiteral("Energy group 0 (mWh)"));
    m_mah1Value = addRow(form, QStringLiteral("Capacity group 1 (mAh)"));
    m_mwh1Value = addRow(form, QStringLiteral("Energy group 1 (mWh)"));
    m_temperatureValue = addRow(form, QStringLiteral("Temperature"));
    m_dplusValue = addRow(form, QStringLiteral("D+ voltage"));
    m_dminusValue = addRow(form, QStringLiteral("D- voltage"));

    return group;
}

QLabel *MainWindow::addRow(QFormLayout *form, const QString &title)
{
    QLabel *value = makeValueLabel();
    form->addRow(title + QStringLiteral(":"), value);
    return value;
}

void MainWindow::refreshPorts()
{
    const QString previous = m_portCombo->currentData().toString();
    m_portCombo->clear();

    const QVector<SerialPortInfo> ports = SerialPort::enumerate();
    int tc66Index = -1;
    QString tc66Port;

    for (const SerialPortInfo &port : ports) {
        m_portCombo->addItem(QStringLiteral("%1 - %2").arg(port.portName, port.description),
            port.portName);
        if (port.isTc66) {
            tc66Index = m_portCombo->count() - 1;
            tc66Port = port.portName;
        }
    }

    if (tc66Index >= 0) {
        m_portCombo->setCurrentIndex(tc66Index);
    } else if (!previous.isEmpty()) {
        const int index = m_portCombo->findData(previous);
        if (index >= 0)
            m_portCombo->setCurrentIndex(index);
    }

    if (ports.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("No serial ports found."));
    } else if (!tc66Port.isEmpty()) {
        statusBar()->showMessage(
            QStringLiteral("RDTech TC66 detected on %1.").arg(tc66Port));
    } else {
        statusBar()->showMessage(
            QStringLiteral("%1 serial port(s) found.").arg(ports.size()));
    }
}

void MainWindow::toggleConnection()
{
    if (m_device->isOpen()) {
        m_device->close();
        return;
    }

    const QString port = m_portCombo->currentData().toString();
    if (port.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("Select a serial port first."));
        return;
    }

    m_device->open(port);
}

void MainWindow::onConnectionChanged(bool connected)
{
    m_portCombo->setEnabled(!connected);
    m_refreshButton->setEnabled(!connected);
    m_connectButton->setText(connected ? QStringLiteral("Disconnect") : QStringLiteral("Connect"));

    if (connected) {
        statusBar()->showMessage(
            QStringLiteral("Connected to %1.").arg(m_device->portName()));
    } else {
        statusBar()->showMessage(QStringLiteral("Disconnected."));
    }
}

void MainWindow::onReadingReady(const Tc66Reading &reading)
{
    m_modelValue->setText(reading.model.isEmpty() ? QStringLiteral("--") : reading.model);
    m_firmwareValue->setText(reading.firmware.isEmpty() ? QStringLiteral("--") : reading.firmware);
    m_serialValue->setText(QString::number(reading.serial).rightJustified(8, QLatin1Char('0')));

    m_voltageValue->setText(QStringLiteral("%1 V").arg(reading.voltage, 0, 'f', 4));
    m_currentValue->setText(QStringLiteral("%1 A").arg(reading.current, 0, 'f', 5));
    m_powerValue->setText(QStringLiteral("%1 W").arg(reading.power, 0, 'f', 4));
    m_resistanceValue->setText(
        QStringLiteral("%1 %2").arg(reading.resistance, 0, 'f', 2).arg(QStringLiteral("\u03A9")));
    m_mah0Value->setText(QStringLiteral("%1 mAh").arg(reading.energyMah0, 0, 'f', 0));
    m_mwh0Value->setText(QStringLiteral("%1 mWh").arg(reading.energyMwh0, 0, 'f', 0));
    m_mah1Value->setText(QStringLiteral("%1 mAh").arg(reading.energyMah1, 0, 'f', 0));
    m_mwh1Value->setText(QStringLiteral("%1 mWh").arg(reading.energyMwh1, 0, 'f', 0));
    m_temperatureValue->setText(
        QStringLiteral("%1 %2").arg(reading.temperature, 0, 'f', 1).arg(QStringLiteral("\u00B0C")));
    m_dplusValue->setText(QStringLiteral("%1 V").arg(reading.dPlus, 0, 'f', 2));
    m_dminusValue->setText(QStringLiteral("%1 V").arg(reading.dMinus, 0, 'f', 2));
}

void MainWindow::onError(const QString &message)
{
    statusBar()->showMessage(message, 5000);
}

void MainWindow::loadColors()
{
    QSettings settings;
    const MeasurementColors defaults = MeasurementColors::defaults();

    m_colors.voltage = settings.value(QStringLiteral("colors/voltage"), defaults.voltage).value<QColor>();
    m_colors.current = settings.value(QStringLiteral("colors/current"), defaults.current).value<QColor>();
    m_colors.power = settings.value(QStringLiteral("colors/power"), defaults.power).value<QColor>();
    m_colors.resistance = settings.value(QStringLiteral("colors/resistance"), defaults.resistance).value<QColor>();
}

void MainWindow::saveColors()
{
    QSettings settings;
    settings.setValue(QStringLiteral("colors/voltage"), m_colors.voltage);
    settings.setValue(QStringLiteral("colors/current"), m_colors.current);
    settings.setValue(QStringLiteral("colors/power"), m_colors.power);
    settings.setValue(QStringLiteral("colors/resistance"), m_colors.resistance);
}

void MainWindow::applyColors()
{
    m_voltageValue->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.voltage.name()));
    m_currentValue->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.current.name()));
    m_powerValue->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.power.name()));
    m_resistanceValue->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.resistance.name()));
}

void MainWindow::openPreferences()
{
    PreferencesDialog dialog(m_colors, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_colors = dialog.colors();
        applyColors();
        saveColors();
    }
}
