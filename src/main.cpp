#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("TC66 Monitor"));
    QApplication::setOrganizationName(QStringLiteral("qc66c"));

    MainWindow window;
    window.resize(520, 640);
    window.show();

    return app.exec();
}
