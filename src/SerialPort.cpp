// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#include "SerialPort.h"

#include <initguid.h>
#include <devguid.h>
#include <setupapi.h>

namespace
{
constexpr DWORD kBaudRate = 115200;
constexpr char kTc66VidPid[] = "VID_28E9&PID_018A";

QString lastErrorMessage(const char *prefix)
{
    const DWORD code = GetLastError();
    return QStringLiteral("%1 (Win32 error %2)").arg(QLatin1String(prefix)).arg(code);
}
}

SerialPort::~SerialPort()
{
    close();
}

bool SerialPort::open(const QString &portName)
{
    close();

    const QString path = QStringLiteral("\\\\.\\") + portName;
    m_handle = CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()),
        GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (m_handle == INVALID_HANDLE_VALUE) {
        m_error = lastErrorMessage("Failed to open serial port");
        return false;
    }

    SetupComm(m_handle, 4096, 4096);

    DCB dcb = {};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(m_handle, &dcb)) {
        m_error = lastErrorMessage("GetCommState failed");
        close();
        return false;
    }

    dcb.BaudRate = kBaudRate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fAbortOnError = FALSE;

    if (!SetCommState(m_handle, &dcb)) {
        m_error = lastErrorMessage("SetCommState failed");
        close();
        return false;
    }

    COMMTIMEOUTS timeouts = {};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 100;
    SetCommTimeouts(m_handle, &timeouts);

    PurgeComm(m_handle, PURGE_RXCLEAR | PURGE_TXCLEAR);

    m_error.clear();
    return true;
}

void SerialPort::close()
{
    if (m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

qint64 SerialPort::write(const QByteArray &data)
{
    if (!isOpen())
        return -1;

    DWORD written = 0;
    if (!WriteFile(m_handle, data.constData(), static_cast<DWORD>(data.size()),
            &written, nullptr)) {
        m_error = lastErrorMessage("WriteFile failed");
        return -1;
    }
    return static_cast<qint64>(written);
}

QByteArray SerialPort::readAll()
{
    QByteArray result;
    if (!isOpen())
        return result;

    DWORD errors = 0;
    COMSTAT status = {};
    if (!ClearCommError(m_handle, &errors, &status)) {
        m_error = lastErrorMessage("ClearCommError failed");
        return result;
    }

    if (status.cbInQue == 0)
        return result;

    result.resize(static_cast<int>(status.cbInQue));
    DWORD read = 0;
    if (!ReadFile(m_handle, result.data(), status.cbInQue, &read, nullptr)) {
        m_error = lastErrorMessage("ReadFile failed");
        return QByteArray();
    }

    result.resize(static_cast<int>(read));
    return result;
}

QVector<SerialPortInfo> SerialPort::enumerate()
{
    QVector<SerialPortInfo> ports;

    HDEVINFO deviceInfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
    if (deviceInfo == INVALID_HANDLE_VALUE)
        return ports;

    SP_DEVINFO_DATA info = {};
    info.cbSize = sizeof(info);

    for (DWORD index = 0; SetupDiEnumDeviceInfo(deviceInfo, index, &info); ++index) {
        HKEY key = SetupDiOpenDevRegKey(deviceInfo, &info, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        if (key == INVALID_HANDLE_VALUE)
            continue;

        wchar_t portName[256] = {};
        DWORD size = sizeof(portName);
        DWORD type = 0;
        const LONG query = RegQueryValueExW(key, L"PortName", nullptr, &type,
            reinterpret_cast<LPBYTE>(portName), &size);
        RegCloseKey(key);

        if (query != ERROR_SUCCESS || portName[0] == L'\0')
            continue;

        SerialPortInfo port;
        port.portName = QString::fromWCharArray(portName);

        wchar_t value[512] = {};
        if (SetupDiGetDeviceRegistryPropertyW(deviceInfo, &info, SPDRP_FRIENDLYNAME, nullptr,
                reinterpret_cast<PBYTE>(value), sizeof(value), nullptr)) {
            port.description = QString::fromWCharArray(value);
        }
        if (port.description.isEmpty())
            port.description = port.portName;

        wchar_t hardwareIds[1024] = {};
        if (SetupDiGetDeviceRegistryPropertyW(deviceInfo, &info, SPDRP_HARDWAREID, nullptr,
                reinterpret_cast<PBYTE>(hardwareIds), sizeof(hardwareIds), nullptr)) {
            port.hardwareId = QString::fromWCharArray(hardwareIds);
        }

        port.isTc66 = port.hardwareId.contains(QLatin1String(kTc66VidPid), Qt::CaseInsensitive);

        ports.append(port);
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);
    return ports;
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
