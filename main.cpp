#include "mainwindow.h"
#include <QApplication>
#include "p2token.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName(QStringLiteral("Propeller2-Emulator"));
    a.setApplicationVersion(QString("%1.%2.%3").arg(VER_MAJ).arg(VER_MIN).arg(VER_PAT));
    a.setOrganizationName(QStringLiteral("PullMoll"));
    a.setOrganizationDomain(QStringLiteral("https://propeller2.voidlinux.de/"));

    MainWindow w;
    w.show();

    return a.exec();
}
