#include "PreferencesDialog.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QPushButton>
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

PreferencesDialog::PreferencesDialog(const MeasurementColors &colors, QWidget *parent)
    : QDialog(parent)
    , m_colors(colors)
{
    setWindowTitle(QStringLiteral("Preferences"));

    auto *form = new QFormLayout;

    m_voltageButton = createColorButton(QStringLiteral("Voltage"), m_colors.voltage);
    m_currentButton = createColorButton(QStringLiteral("Current"), m_colors.current);
    m_powerButton = createColorButton(QStringLiteral("Power"), m_colors.power);
    m_resistanceButton = createColorButton(QStringLiteral("Resistance"), m_colors.resistance);

    form->addRow(QStringLiteral("Voltage:"), m_voltageButton);
    form->addRow(QStringLiteral("Current:"), m_currentButton);
    form->addRow(QStringLiteral("Power:"), m_powerButton);
    form->addRow(QStringLiteral("Resistance:"), m_resistanceButton);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *resetButton = buttons->addButton(QStringLiteral("Restore Defaults"),
        QDialogButtonBox::ResetRole);
    connect(resetButton, &QPushButton::clicked, this, &PreferencesDialog::restoreDefaults);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

MeasurementColors PreferencesDialog::colors() const
{
    return m_colors;
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

void PreferencesDialog::restoreDefaults()
{
    m_colors = MeasurementColors::defaults();
    updateSwatch(m_voltageButton, m_colors.voltage);
    updateSwatch(m_currentButton, m_colors.current);
    updateSwatch(m_powerButton, m_colors.power);
    updateSwatch(m_resistanceButton, m_colors.resistance);
}
