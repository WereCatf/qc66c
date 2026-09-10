// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026 Nita Vesa

#include <QApplication>
#include <QIcon>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Qc66c"));
    QApplication::setOrganizationName(QStringLiteral("qc66c"));
#ifdef APP_VERSION
    QApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
#endif
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/qc66c.ico")));

    MainWindow window;
    window.show();

    return app.exec();
}
