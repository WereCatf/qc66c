// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#pragma once

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QTimer>

#include "SerialPort.h"
#include "Tc66Protocol.h"

class Tc66Device : public QObject
{
    Q_OBJECT

public:
    explicit Tc66Device(QObject *parent = nullptr);

    bool open(const QString &portName);
    void close();
    bool isOpen() const { return m_serial.isOpen(); }
    QString portName() const { return m_portName; }

signals:
    void readingReady(const Tc66Reading &reading);
    void connectionChanged(bool connected);
    void errorOccurred(const QString &message);

private slots:
    void poll();
    void readAvailable();

private:
    void processBuffer();

    SerialPort m_serial;
    QTimer m_pollTimer;
    QTimer m_readTimer;
    QByteArray m_buffer;
    QElapsedTimer m_sincePoll;
    QString m_portName;
    bool m_waitingForResponse = false;
};
