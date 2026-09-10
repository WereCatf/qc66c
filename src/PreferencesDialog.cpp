// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#include "PreferencesDialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

MeasurementColors MeasurementColors::defaults()
{
    MeasurementColors colors;
    colors.voltage = QColor(QStringLiteral("#1E88E5"));
    colors.current = QColor(QStringLiteral("#E53935"));
    colors.power = QColor(QStringLiteral("#43A047"));
    colors.resistance = QColor(QStringLiteral("#FB8C00"));
    return colors;
}

MeasurementVisibility MeasurementVisibility::defaults()
{
    return MeasurementVisibility();
}

AppSettings AppSettings::defaults()
{
    AppSettings settings;
    settings.autoConnect = false;
    settings.colors = MeasurementColors::defaults();
    settings.visibility = MeasurementVisibility::defaults();

    QFont font(QStringLiteral("Consolas"));
    font.setStyleHint(QFont::Monospace);
    font.setBold(true);
    int pointSize = QApplication::font().pointSize();
    if (pointSize <= 0)
        pointSize = 10;
    font.setPointSize(pointSize + 1);
    settings.font = font;

    return settings;
}

PreferencesDialog::PreferencesDialog(const AppSettings &settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(QStringLiteral("Preferences"));

    auto *tabs = new QTabWidget(this);
    tabs->addTab(buildGeneralTab(), QStringLiteral("General"));
    tabs->addTab(buildMeasurementsTab(), QStringLiteral("Measurements"));
    tabs->addTab(buildFontTab(), QStringLiteral("Font"));
    tabs->addTab(buildColoursTab(), QStringLiteral("Colours"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *resetButton = buttons->addButton(QStringLiteral("Restore Defaults"),
        QDialogButtonBox::ResetRole);
    connect(resetButton, &QPushButton::clicked, this, &PreferencesDialog::restoreDefaults);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    layout->addWidget(buttons);
}

QWidget *PreferencesDialog::buildGeneralTab()
{
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);

    m_autoConnectCheck = new QCheckBox(
        QStringLiteral("Connect automatically on start-up when the TC66 port is available"), tab);
    m_autoConnectCheck->setChecked(m_settings.autoConnect);

    layout->addWidget(m_autoConnectCheck);
    layout->addStretch();

    return tab;
}

QWidget *PreferencesDialog::buildMeasurementsTab()
{
    auto *tab = new QWidget(this);
    auto *grid = new QGridLayout(tab);

    const auto makeCheck = [this](const QString &text, bool checked) {
        auto *check = new QCheckBox(text, this);
        check->setChecked(checked);
        return check;
    };

    m_voltageCheck = makeCheck(QStringLiteral("Voltage"), m_settings.visibility.voltage);
    m_currentCheck = makeCheck(QStringLiteral("Current"), m_settings.visibility.current);
    m_powerCheck = makeCheck(QStringLiteral("Power"), m_settings.visibility.power);
    m_resistanceCheck = makeCheck(QStringLiteral("Resistance"), m_settings.visibility.resistance);
    m_capacity0Check = makeCheck(QStringLiteral("Capacity group 0 (mAh)"), m_settings.visibility.capacity0);
    m_energy0Check = makeCheck(QStringLiteral("Energy group 0 (mWh)"), m_settings.visibility.energy0);
    m_capacity1Check = makeCheck(QStringLiteral("Capacity group 1 (mAh)"), m_settings.visibility.capacity1);
    m_energy1Check = makeCheck(QStringLiteral("Energy group 1 (mWh)"), m_settings.visibility.energy1);
    m_temperatureCheck = makeCheck(QStringLiteral("Temperature"), m_settings.visibility.temperature);
    m_dplusCheck = makeCheck(QStringLiteral("D+ voltage"), m_settings.visibility.dPlus);
    m_dminusCheck = makeCheck(QStringLiteral("D- voltage"), m_settings.visibility.dMinus);

    QCheckBox *left[] = { m_voltageCheck, m_currentCheck, m_powerCheck, m_resistanceCheck,
        m_capacity0Check, m_energy0Check };
    QCheckBox *right[] = { m_capacity1Check, m_energy1Check, m_temperatureCheck,
        m_dplusCheck, m_dminusCheck };

    for (int row = 0; row < 6; ++row)
        grid->addWidget(left[row], row, 0);
    for (int row = 0; row < 5; ++row)
        grid->addWidget(right[row], row, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    return tab;
}

QWidget *PreferencesDialog::buildFontTab()
{
    auto *tab = new QWidget(this);
    auto *form = new QFormLayout(tab);

    m_fontCombo = new QFontComboBox(tab);
    m_fontCombo->setCurrentFont(m_settings.font);

    m_fontSize = new QSpinBox(tab);
    m_fontSize->setRange(6, 72);
    m_fontSize->setValue(m_settings.font.pointSize() > 0 ? m_settings.font.pointSize() : 11);

    m_fontBold = new QCheckBox(QStringLiteral("Bold"), tab);
    m_fontBold->setChecked(m_settings.font.bold());

    m_fontPreview = new QLabel(QStringLiteral("1234.5678 V  1.23456 A"), tab);
    m_fontPreview->setMinimumHeight(48);

    form->addRow(QStringLiteral("Font:"), m_fontCombo);
    form->addRow(QStringLiteral("Size:"), m_fontSize);
    form->addRow(QString(), m_fontBold);
    form->addRow(QStringLiteral("Preview:"), m_fontPreview);

    connect(m_fontCombo, &QFontComboBox::currentFontChanged, this, &PreferencesDialog::updateFontPreview);
    connect(m_fontSize, &QSpinBox::valueChanged, this, &PreferencesDialog::updateFontPreview);
    connect(m_fontBold, &QCheckBox::toggled, this, &PreferencesDialog::updateFontPreview);

    updateFontPreview();
    return tab;
}

QWidget *PreferencesDialog::buildColoursTab()
{
    auto *tab = new QWidget(this);
    auto *form = new QFormLayout(tab);

    m_voltageButton = createColorButton(QStringLiteral("Voltage"), m_settings.colors.voltage);
    m_currentButton = createColorButton(QStringLiteral("Current"), m_settings.colors.current);
    m_powerButton = createColorButton(QStringLiteral("Power"), m_settings.colors.power);
    m_resistanceButton = createColorButton(QStringLiteral("Resistance"), m_settings.colors.resistance);

    form->addRow(QStringLiteral("Voltage:"), m_voltageButton);
    form->addRow(QStringLiteral("Current:"), m_currentButton);
    form->addRow(QStringLiteral("Power:"), m_powerButton);
    form->addRow(QStringLiteral("Resistance:"), m_resistanceButton);

    return tab;
}

AppSettings PreferencesDialog::settings() const
{
    AppSettings result = m_settings;

    result.autoConnect = m_autoConnectCheck->isChecked();

    result.visibility.voltage = m_voltageCheck->isChecked();
    result.visibility.current = m_currentCheck->isChecked();
    result.visibility.power = m_powerCheck->isChecked();
    result.visibility.resistance = m_resistanceCheck->isChecked();
    result.visibility.capacity0 = m_capacity0Check->isChecked();
    result.visibility.energy0 = m_energy0Check->isChecked();
    result.visibility.capacity1 = m_capacity1Check->isChecked();
    result.visibility.energy1 = m_energy1Check->isChecked();
    result.visibility.temperature = m_temperatureCheck->isChecked();
    result.visibility.dPlus = m_dplusCheck->isChecked();
    result.visibility.dMinus = m_dminusCheck->isChecked();

    result.font = m_fontCombo->currentFont();
    result.font.setPointSize(m_fontSize->value());
    result.font.setBold(m_fontBold->isChecked());

    return result;
}

QPushButton *PreferencesDialog::createColorButton(const QString &title, QColor &color)
{
    auto *button = new QPushButton(this);
    button->setMinimumWidth(120);
    button->setCursor(Qt::PointingHandCursor);
    connect(button, &QPushButton::clicked, this, [this, title, &color, button] {
        chooseColor(title, color, button);
    });
    updateSwatch(button, color);
    return button;
}

void PreferencesDialog::updateSwatch(QPushButton *button, const QColor &color)
{
    const QString textColor = color.lightness() < 128 ? QStringLiteral("#FFFFFF")
                                                       : QStringLiteral("#000000");
    button->setText(color.name().toUpper());
    button->setStyleSheet(
        QStringLiteral("background-color: %1; color: %2; border: 1px solid #888; padding: 4px;")
            .arg(color.name(), textColor));
}

void PreferencesDialog::chooseColor(const QString &title, QColor &color, QPushButton *button)
{
    const QColor chosen = QColorDialog::getColor(color, this,
        QStringLiteral("Select %1 colour").arg(title));
    if (chosen.isValid()) {
        color = chosen;
        updateSwatch(button, color);
    }
}

void PreferencesDialog::updateFontPreview()
{
    QFont font = m_fontCombo->currentFont();
    font.setPointSize(m_fontSize->value());
    font.setBold(m_fontBold->isChecked());
    m_fontPreview->setFont(font);
}

void PreferencesDialog::restoreDefaults()
{
    m_settings = AppSettings::defaults();

    m_autoConnectCheck->setChecked(m_settings.autoConnect);

    m_voltageCheck->setChecked(m_settings.visibility.voltage);
    m_currentCheck->setChecked(m_settings.visibility.current);
    m_powerCheck->setChecked(m_settings.visibility.power);
    m_resistanceCheck->setChecked(m_settings.visibility.resistance);
    m_capacity0Check->setChecked(m_settings.visibility.capacity0);
    m_energy0Check->setChecked(m_settings.visibility.energy0);
    m_capacity1Check->setChecked(m_settings.visibility.capacity1);
    m_energy1Check->setChecked(m_settings.visibility.energy1);
    m_temperatureCheck->setChecked(m_settings.visibility.temperature);
    m_dplusCheck->setChecked(m_settings.visibility.dPlus);
    m_dminusCheck->setChecked(m_settings.visibility.dMinus);

    m_fontCombo->setCurrentFont(m_settings.font);
    m_fontSize->setValue(m_settings.font.pointSize());
    m_fontBold->setChecked(m_settings.font.bold());
    updateFontPreview();

    updateSwatch(m_voltageButton, m_settings.colors.voltage);
    updateSwatch(m_currentButton, m_settings.colors.current);
    updateSwatch(m_powerButton, m_settings.colors.power);
    updateSwatch(m_resistanceButton, m_settings.colors.resistance);
}
