#pragma once

#include <QMainWindow>

#include "PreferencesDialog.h"
#include "Tc66Protocol.h"

class QComboBox;
class QFormLayout;
class QLabel;
class QPushButton;
class Tc66Device;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void refreshPorts();
    void toggleConnection();
    void onConnectionChanged(bool connected);
    void onReadingReady(const Tc66Reading &reading);
    void onError(const QString &message);
    void openPreferences();

private:
    QWidget *buildConnectionGroup();
    QWidget *buildDeviceGroup();
    QWidget *buildReadingsGroup();
    QLabel *addRow(QFormLayout *form, const QString &title);

    void loadSettings();
    void saveSettings();
    void applyAppearance();

    Tc66Device *m_device = nullptr;
    QComboBox *m_portCombo = nullptr;
    QPushButton *m_refreshButton = nullptr;
    QPushButton *m_connectButton = nullptr;
    QFormLayout *m_readingsForm = nullptr;

    QLabel *m_modelValue = nullptr;
    QLabel *m_firmwareValue = nullptr;
    QLabel *m_serialValue = nullptr;

    QLabel *m_voltageValue = nullptr;
    QLabel *m_currentValue = nullptr;
    QLabel *m_powerValue = nullptr;
    QLabel *m_resistanceValue = nullptr;
    QLabel *m_mah0Value = nullptr;
    QLabel *m_mwh0Value = nullptr;
    QLabel *m_mah1Value = nullptr;
    QLabel *m_mwh1Value = nullptr;
    QLabel *m_temperatureValue = nullptr;
    QLabel *m_dplusValue = nullptr;
    QLabel *m_dminusValue = nullptr;

    AppearanceSettings m_appearance;
};
