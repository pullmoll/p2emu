#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName(QStringLiteral("Propeller2 Emulator"));
    a.setApplicationVersion(QStringLiteral("0.0.1"));
    a.setOrganizationName(QStringLiteral("PullMoll"));
    a.setOrganizationDomain(QStringLiteral("https://propeller2.voidlinux.de/"));

    MainWindow w;
    w.show();

    return a.exec();
}
