// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#include "Tc66Protocol.h"

#include <cstring>

extern "C" {
#include "aes.h"
}

namespace
{
// Static AES-256 key used by the TC66C poll response. Documented at
// https://sigrok.org/wiki/RDTech_TC66C and in libsigrok's rdtech-tc driver.
const uint8_t kAesKey[32] = {
    0x58, 0x21, 0xfa, 0x56, 0x01, 0xb2, 0xf0, 0x26,
    0x87, 0xff, 0x12, 0x04, 0x62, 0x2a, 0x4f, 0xb0,
    0x86, 0xf4, 0x02, 0x60, 0x81, 0x6f, 0x9a, 0x0b,
    0xa7, 0xf1, 0x06, 0x61, 0x9a, 0xb8, 0x72, 0x88,
};

constexpr int PacLen = 64;
constexpr int PacCrcPos = PacLen - 4;
constexpr int PacketLen = 3 * PacLen;

quint32 readU32Le(const uint8_t *data)
{
    return static_cast<quint32>(data[0])
        | (static_cast<quint32>(data[1]) << 8)
        | (static_cast<quint32>(data[2]) << 16)
        | (static_cast<quint32>(data[3]) << 24);
}

bool checkPac(const QByteArray &data, int blockOffset, const char *magic)
{
    const uint8_t *block = reinterpret_cast<const uint8_t *>(data.constData()) + blockOffset;
    if (std::memcmp(block, magic, 4) != 0)
        return false;

    const quint16 calculated = Tc66Protocol::crc16Modbus(block, PacCrcPos);
    const quint32 stored = readU32Le(block + PacCrcPos);
    return calculated == static_cast<quint16>(stored);
}
}

namespace Tc66Protocol
{

quint16 crc16Modbus(const uint8_t *data, int len)
{
    quint16 crc = 0xFFFF;
    for (int i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 0x0001)
                crc = static_cast<quint16>((crc >> 1) ^ 0xA001);
            else
                crc >>= 1;
        }
    }
    return crc;
}

bool decrypt(const QByteArray &encrypted, QByteArray &decrypted)
{
    if (encrypted.size() != PacketSize)
        return false;

    decrypted = encrypted;
    auto *bytes = reinterpret_cast<uint8_t *>(decrypted.data());

    struct AES_ctx ctx;
    AES_init_ctx(&ctx, kAesKey);
    for (int offset = 0; offset < PacketLen; offset += AES_BLOCKLEN)
        AES_ECB_decrypt(&ctx, bytes + offset);

    return true;
}

bool parse(const QByteArray &decrypted, Tc66Reading &reading)
{
    if (decrypted.size() < PacketLen)
        return false;

    if (!checkPac(decrypted, 0 * PacLen, "pac1")
        || !checkPac(decrypted, 1 * PacLen, "pac2")
        || !checkPac(decrypted, 2 * PacLen, "pac3"))
        return false;

    const uint8_t *d = reinterpret_cast<const uint8_t *>(decrypted.constData());

    reading.valid = true;
    reading.model = QString::fromLatin1(reinterpret_cast<const char *>(d + 4), 4).trimmed();
    reading.firmware = QString::fromLatin1(reinterpret_cast<const char *>(d + 8), 4).trimmed();
    reading.serial = readU32Le(d + 12);

    reading.voltage = readU32Le(d + 48) / 1.0e4;
    reading.current = readU32Le(d + 52) / 1.0e5;
    reading.power = readU32Le(d + 56) / 1.0e4;

    reading.resistance = readU32Le(d + PacLen + 4) / 1.0e2;
    reading.energyMah0 = readU32Le(d + PacLen + 8);
    reading.energyMwh0 = readU32Le(d + PacLen + 12);
    reading.energyMah1 = readU32Le(d + PacLen + 16);
    reading.energyMwh1 = readU32Le(d + PacLen + 20);

    const quint32 temperatureSign = readU32Le(d + PacLen + 24);
    const double temperature = readU32Le(d + PacLen + 28);
    reading.temperature = temperatureSign == 1 ? -temperature : temperature;

    reading.dPlus = readU32Le(d + PacLen + 32) / 1.0e2;
    reading.dMinus = readU32Le(d + PacLen + 36) / 1.0e2;

    return true;
}

}
