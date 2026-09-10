#pragma once

#include <QColor>
#include <QDialog>

class QPushButton;

struct MeasurementColors
{
    QColor voltage;
    QColor current;
    QColor power;
    QColor resistance;

    static MeasurementColors defaults();
};

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(const MeasurementColors &colors, QWidget *parent = nullptr);

    MeasurementColors colors() const;

private:
    QPushButton *createColorButton(const QString &title, QColor &color);
    void updateSwatch(QPushButton *button, const QColor &color);
    void chooseColor(const QString &title, QColor &color, QPushButton *button);
    void restoreDefaults();

    MeasurementColors m_colors;
    QPushButton *m_voltageButton = nullptr;
    QPushButton *m_currentButton = nullptr;
    QPushButton *m_powerButton = nullptr;
    QPushButton *m_resistanceButton = nullptr;
};
