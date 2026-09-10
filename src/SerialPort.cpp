// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#include "SerialPort.h"

SerialPort::~SerialPort()
{
    close();
}

QString SerialPort::findTc66Port()
{
    const QVector<SerialPortInfo> ports = enumerate();
    for (const SerialPortInfo &port : ports) {
        if (port.isTc66)
            return port.portName;
    }
    return QString();
}
