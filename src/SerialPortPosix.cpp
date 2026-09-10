// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#include "SerialPort.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

namespace
{
constexpr speed_t kBaudRate = B115200;
constexpr char kTc66Vid[] = "28e9";
constexpr char kTc66Pid[] = "018a";

QString errnoMessage(const char *prefix)
{
    return QStringLiteral("%1 (%2)")
        .arg(QLatin1String(prefix), QString::fromLocal8Bit(std::strerror(errno)));
}

QString readSysfsFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromLatin1(file.readAll()).trimmed();
}

// For a tty like ttyACM0/ttyUSB0, the USB device directory sits above the
// resolved /sys/class/tty/<name>/device symlink. Walk up looking for the
// requested attribute (idVendor, idProduct, product, ...).
QString usbAttribute(const QString &ttyName, const QString &attribute)
{
    const QFileInfo link(QStringLiteral("/sys/class/tty/%1/device").arg(ttyName));
    if (!link.exists())
        return QString();

    QString dir = link.canonicalFilePath();
    for (int depth = 0; depth < 8 && !dir.isEmpty(); ++depth) {
        const QString candidate = dir + QLatin1Char('/') + attribute;
        if (QFileInfo::exists(candidate))
            return readSysfsFile(candidate);

        QDir parent(dir);
        if (!parent.cdUp())
            break;
        dir = parent.absolutePath();
        if (dir == QLatin1String("/sys") || dir == QLatin1String("/"))
            break;
    }
    return QString();
}

QString describePort(const QString &ttyName)
{
    const QString product = usbAttribute(ttyName, QStringLiteral("product"));
    const QString manufacturer = usbAttribute(ttyName, QStringLiteral("manufacturer"));

    if (!manufacturer.isEmpty() && !product.isEmpty())
        return QStringLiteral("%1 %2").arg(manufacturer, product);
    if (!product.isEmpty())
        return product;
    if (!manufacturer.isEmpty())
        return manufacturer;
    return ttyName;
}
}

bool SerialPort::open(const QString &portName)
{
    close();

    const QByteArray path = portName.toLocal8Bit();
    const int fd = ::open(path.constData(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        m_error = errnoMessage("Failed to open serial port");
        return false;
    }
    m_handle = fd;

    termios tty = {};
    if (tcgetattr(fd, &tty) != 0) {
        m_error = errnoMessage("tcgetattr failed");
        close();
        return false;
    }

    cfmakeraw(&tty);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (cfsetispeed(&tty, kBaudRate) != 0 || cfsetospeed(&tty, kBaudRate) != 0) {
        m_error = errnoMessage("Failed to set baud rate");
        close();
        return false;
    }

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        m_error = errnoMessage("tcsetattr failed");
        close();
        return false;
    }

    tcflush(fd, TCIOFLUSH);

    m_error.clear();
    return true;
}

void SerialPort::close()
{
    if (m_handle != kInvalidSerialHandle) {
        ::close(m_handle);
        m_handle = kInvalidSerialHandle;
    }
}

qint64 SerialPort::write(const QByteArray &data)
{
    if (!isOpen())
        return -1;

    const ssize_t written = ::write(m_handle, data.constData(), static_cast<size_t>(data.size()));
    if (written < 0) {
        m_error = errnoMessage("write failed");
        return -1;
    }
    return static_cast<qint64>(written);
}

QByteArray SerialPort::readAll()
{
    QByteArray result;
    if (!isOpen())
        return result;

    char buffer[4096];
    for (;;) {
        const ssize_t n = ::read(m_handle, buffer, sizeof(buffer));
        if (n > 0) {
            result.append(buffer, static_cast<int>(n));
            continue;
        }
        if (n < 0 && errno == EINTR)
            continue;
        break; // no more data (EAGAIN) or error
    }
    return result;
}

QVector<SerialPortInfo> SerialPort::enumerate()
{
    QVector<SerialPortInfo> ports;

    const QDir devDir(QStringLiteral("/dev"));
    const QStringList names = devDir.entryList(
        { QStringLiteral("ttyACM*"), QStringLiteral("ttyUSB*") },
        QDir::AllEntries | QDir::System, QDir::Name);

    for (const QString &name : names) {
        const QString vid = usbAttribute(name, QStringLiteral("idVendor")).toLower();
        const QString pid = usbAttribute(name, QStringLiteral("idProduct")).toLower();

        SerialPortInfo port;
        port.portName = QStringLiteral("/dev/%1").arg(name);
        port.description = describePort(name);
        if (!vid.isEmpty() && !pid.isEmpty()) {
            port.hardwareId = QStringLiteral("USB\\VID_%1&PID_%2")
                                  .arg(vid.toUpper(), pid.toUpper());
        }
        port.isTc66 = (vid == QLatin1String(kTc66Vid) && pid == QLatin1String(kTc66Pid));

        ports.append(port);
    }

    return ports;
}
