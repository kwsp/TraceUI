#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>

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
#endif

    QQmlApplicationEngine engine;

#ifdef Q_OS_MACOS
    engine.rootContext()->setContextProperty("systemMonitor", &systemMonitor);
    engine.rootContext()->setContextProperty("processWatcher", &processWatcher);
    engine.rootContext()->setContextProperty("networkMonitor", &networkMonitor);
#endif

    engine.loadFromModule(APP_MODULE_NAME, "Main");

    return QGuiApplication::exec();
}
