// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#pragma once

#include <QByteArray>
#include <QString>
#include <QVector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

struct SerialPortInfo
{
    QString portName;    // e.g. "COM3"
    QString description; // human readable name
    QString hardwareId;  // first hardware id reported by Windows
    bool isTc66 = false; // matches VID_28E9 & PID_018A
};

// Minimal blocking serial-port wrapper built on the Win32 comm API.
// Qt's SerialPort module is not required.
class SerialPort
{
public:
    SerialPort() = default;
    ~SerialPort();

    SerialPort(const SerialPort &) = delete;
    SerialPort &operator=(const SerialPort &) = delete;

    bool open(const QString &portName);
    void close();
    bool isOpen() const { return m_handle != INVALID_HANDLE_VALUE; }

    qint64 write(const QByteArray &data);
    QByteArray readAll();

    QString errorString() const { return m_error; }

    static QVector<SerialPortInfo> enumerate();
    static QString findTc66Port();

private:
    HANDLE m_handle = INVALID_HANDLE_VALUE;
    QString m_error;
};
