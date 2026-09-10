// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#pragma once

#include <QByteArray>
#include <QString>
#include <cstdint>

struct Tc66Reading
{
    bool valid = false;
    QString model;
    QString firmware;
    quint32 serial = 0;
    double voltage = 0.0;    // V
    double current = 0.0;    // A
    double power = 0.0;      // W
    double resistance = 0.0; // Ohm
    double energyMah0 = 0.0;
    double energyMwh0 = 0.0;
    double energyMah1 = 0.0;
    double energyMwh1 = 0.0;
    double temperature = 0.0; // degrees C
    double dPlus = 0.0;       // V
    double dMinus = 0.0;      // V
};

namespace Tc66Protocol
{
constexpr int PacketSize = 192;

quint16 crc16Modbus(const uint8_t *data, int len);

bool decrypt(const QByteArray &encrypted, QByteArray &decrypted);
bool parse(const QByteArray &decrypted, Tc66Reading &reading);
}
