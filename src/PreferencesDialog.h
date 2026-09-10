// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#pragma once

#include <QColor>
#include <QDialog>
#include <QFont>

class QCheckBox;
class QFontComboBox;
class QLabel;
class QPushButton;
class QSpinBox;

struct MeasurementColors
{
    QColor voltage;
    QColor current;
    QColor power;
    QColor resistance;

    static MeasurementColors defaults();
};

struct MeasurementVisibility
{
    bool voltage = true;
    bool current = true;
    bool power = true;
    bool resistance = true;
    bool capacity0 = true;
    bool energy0 = true;
    bool capacity1 = true;
    bool energy1 = true;
    bool temperature = true;
    bool dPlus = true;
    bool dMinus = true;

    static MeasurementVisibility defaults();
};

struct AppSettings
{
    bool autoConnect = false;
    MeasurementColors colors;
    MeasurementVisibility visibility;
    QFont font;

    static AppSettings defaults();
};

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(const AppSettings &settings, QWidget *parent = nullptr);

    AppSettings settings() const;

private:
    QWidget *buildGeneralTab();
    QWidget *buildMeasurementsTab();
    QWidget *buildFontTab();
    QWidget *buildColoursTab();

    QPushButton *createColorButton(const QString &title, QColor &color);
    void updateSwatch(QPushButton *button, const QColor &color);
    void chooseColor(const QString &title, QColor &color, QPushButton *button);
    void updateFontPreview();
    void restoreDefaults();

    AppSettings m_settings;

    QCheckBox *m_autoConnectCheck = nullptr;

    QCheckBox *m_voltageCheck = nullptr;
    QCheckBox *m_currentCheck = nullptr;
    QCheckBox *m_powerCheck = nullptr;
    QCheckBox *m_resistanceCheck = nullptr;
    QCheckBox *m_capacity0Check = nullptr;
    QCheckBox *m_energy0Check = nullptr;
    QCheckBox *m_capacity1Check = nullptr;
    QCheckBox *m_energy1Check = nullptr;
    QCheckBox *m_temperatureCheck = nullptr;
    QCheckBox *m_dplusCheck = nullptr;
    QCheckBox *m_dminusCheck = nullptr;

    QFontComboBox *m_fontCombo = nullptr;
    QSpinBox *m_fontSize = nullptr;
    QCheckBox *m_fontBold = nullptr;
    QLabel *m_fontPreview = nullptr;

    QPushButton *m_voltageButton = nullptr;
    QPushButton *m_currentButton = nullptr;
    QPushButton *m_powerButton = nullptr;
    QPushButton *m_resistanceButton = nullptr;
};
