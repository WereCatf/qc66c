// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#include "Tc66Device.h"

#include <QDebug>

namespace
{
constexpr int kPollIntervalMs = 100;
constexpr int kReadIntervalMs = 10;
constexpr qint64 kResponseTimeoutMs = 1000;
constexpr int kMaxBufferSize = 8192;
const char kPollCommand[] = "getva";
}

Tc66Device::Tc66Device(QObject *parent)
    : QObject(parent)
{
    m_pollTimer.setInterval(kPollIntervalMs);
    m_readTimer.setInterval(kReadIntervalMs);

    connect(&m_pollTimer, &QTimer::timeout, this, &Tc66Device::poll);
    connect(&m_readTimer, &QTimer::timeout, this, &Tc66Device::readAvailable);
}

bool Tc66Device::open(const QString &portName)
{
    close();

    if (!m_serial.open(portName)) {
        const QString message = m_serial.errorString();
        emit errorOccurred(message);
        return false;
    }

    m_portName = portName;
    m_buffer.clear();
    m_waitingForResponse = false;
    m_sincePoll.start();

    m_readTimer.start();
    m_pollTimer.start();

    poll();
    emit connectionChanged(true);
    return true;
}

void Tc66Device::close()
{
    const bool wasOpen = m_serial.isOpen();
    m_pollTimer.stop();
    m_readTimer.stop();
    m_serial.close();
    m_buffer.clear();
    m_waitingForResponse = false;

    if (wasOpen) {
        m_portName.clear();
        emit connectionChanged(false);
    }
}

void Tc66Device::poll()
{
    if (!m_serial.isOpen())
        return;

    if (m_waitingForResponse) {
        if (m_sincePoll.isValid() && m_sincePoll.elapsed() < kResponseTimeoutMs)
            return;
        // Previous request went unanswered; allow another attempt.
        m_waitingForResponse = false;
    }

    const qint64 written = m_serial.write(QByteArray(kPollCommand, sizeof(kPollCommand) - 1));
    if (written < 0) {
        emit errorOccurred(m_serial.errorString());
        close();
        return;
    }

    m_waitingForResponse = true;
    m_sincePoll.restart();
}

void Tc66Device::readAvailable()
{
    const QByteArray chunk = m_serial.readAll();
    if (!chunk.isEmpty())
        m_buffer.append(chunk);

    if (m_buffer.size() > kMaxBufferSize)
        m_buffer.remove(0, m_buffer.size() - kMaxBufferSize);

    processBuffer();
}

void Tc66Device::processBuffer()
{
    while (m_buffer.size() >= Tc66Protocol::PacketSize) {
        const QByteArray frame = m_buffer.left(Tc66Protocol::PacketSize);

        QByteArray decrypted;
        Tc66Reading reading;
        if (Tc66Protocol::decrypt(frame, decrypted) && Tc66Protocol::parse(decrypted, reading)) {
            m_waitingForResponse = false;
            m_buffer.remove(0, Tc66Protocol::PacketSize);
            emit readingReady(reading);
        } else {
            // Not a valid frame; drop a byte and try to re-synchronise.
            m_buffer.remove(0, 1);
        }
    }
}
