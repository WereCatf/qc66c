// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#pragma once

#include <QByteArray>
#include <QString>
#include <QVector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
using SerialHandle = HANDLE;
inline const SerialHandle kInvalidSerialHandle = INVALID_HANDLE_VALUE;
#else
using SerialHandle = int;
inline const SerialHandle kInvalidSerialHandle = -1;
#endif

struct SerialPortInfo
{
    QString portName;    // e.g. "COM3" or "/dev/ttyACM0"
    QString description; // human readable name
    QString hardwareId;  // e.g. "USB\\VID_28E9&PID_018A"
    bool isTc66 = false; // matches VID 28E9 & PID 018A
};

// Minimal serial-port wrapper. The backend is implemented with the Win32 comm
// API on Windows and POSIX termios elsewhere, so Qt's SerialPort module is not
// required on any platform.
class SerialPort
{
public:
    SerialPort() = default;
    ~SerialPort();

    SerialPort(const SerialPort &) = delete;
    SerialPort &operator=(const SerialPort &) = delete;

    bool open(const QString &portName);
    void close();
    bool isOpen() const { return m_handle != kInvalidSerialHandle; }

    qint64 write(const QByteArray &data);
    QByteArray readAll();

    QString errorString() const { return m_error; }

    static QVector<SerialPortInfo> enumerate();
    static QString findTc66Port();

private:
    SerialHandle m_handle = kInvalidSerialHandle;
    QString m_error;
};
