#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QThread>
#include <QDir>
#include <QtQml>
#include <QMessageBox>
#include <QDebug>

#include "rasdialer.h"
#include "servermodel.h"
#include "ping.h"

int main(int argc, char *argv[])
{
//    qDebug("WinVer: %x. Sizeof: %d", WINVER, sizeof(RASDIALPARAMS));
    QApplication app(argc, argv);
    app.setOrganizationName("Truelancer");
    app.setOrganizationDomain("truelancer.com");
    app.setApplicationName("VPN PPTP Connection Manager V1.0");

    QDir appDir(app.applicationDirPath());

    QFileInfo serversFile(appDir.absoluteFilePath("servers.xml"));
    QFileInfo phonebookFile(appDir.absoluteFilePath("phonebook.pbk"));
    auto ensureFile = [](const QFileInfo &fi)
    {
        if (!fi.exists()) {
            QFile file(QString(":/%1").arg(fi.fileName()));
            bool ok = file.copy(fi.absoluteFilePath());
            if (!ok) {
                QMessageBox::critical(nullptr, qApp->applicationName(),
                                      QString("Failed to create file: %1")
                                      .arg(fi.absoluteFilePath()));
                return false;
            }
        }
        return true;
    };
    if (!ensureFile(serversFile) || !ensureFile(phonebookFile))
        return EXIT_FAILURE;

    qmlRegisterType<ServerModel>("RasDialer", 1, 0, "ServerModel");
    qmlRegisterType<RasDialer>("RasDialer", 1, 0, "RasDialer");
    qmlRegisterType<Ping>("RasDialer", 1, 0, "Ping");

    RasDialer dialer;
    ServerModel model(QDir(app.applicationDirPath()).filePath("servers.xml"));

    QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("dialer", &dialer);
    engine.rootContext()->setContextProperty("serverModel", &model);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

	return app.exec();
}
