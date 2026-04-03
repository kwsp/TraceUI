#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtQml/qqml.h>

#ifdef Q_OS_MACOS
#include "backend/macos/SystemMonitorMacOS.h"
#include "backend/macos/ProcessWatcherMacOS.h"
#include "backend/macos/NetworkMonitorMacOS.h"
#endif

#include "config.h"

int main(int argc, char *argv[]) {
    QQuickWindow::setDefaultAlphaBuffer(true);
    QGuiApplication app(argc, argv);

#ifdef Q_OS_MACOS
    SystemMonitorMacOS systemMonitor;
    ProcessWatcherMacOS processWatcher;
    NetworkMonitorMacOS networkMonitor;

    qmlRegisterSingletonInstance<SystemMonitor> ("TraceUI", 0, 1, "SystemMonitor",  &systemMonitor);
    qmlRegisterSingletonInstance<ProcessWatcher>("TraceUI", 0, 1, "ProcessWatcher", &processWatcher);
    qmlRegisterSingletonInstance<NetworkMonitor>("TraceUI", 0, 1, "NetworkMonitor", &networkMonitor);
#endif

    QQmlApplicationEngine engine;
    engine.loadFromModule(APP_MODULE_NAME, "Main");

    return QGuiApplication::exec();
}
