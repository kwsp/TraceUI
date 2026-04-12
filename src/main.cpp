#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QtQml/qqml.h>

#include "backend/CpuHistoryProvider.h"
#include "backend/NetworkHistoryProvider.h"
#include "config.h"

#ifdef Q_OS_MACOS
#include "backend/macos/NetworkMonitorMacOS.h"
#include "backend/macos/ProcessWatcherMacOS.h"
#include "backend/macos/SystemMonitorMacOS.h"
#endif

#include "config.h"

#include <QSurfaceFormat>
#include <QtQml/qqmlextensionplugin.h>

// Import the static EncomGlobe QML plugin
Q_IMPORT_QML_PLUGIN(EncomGlobePlugin)

#if BUILD_TERMINAL
Q_IMPORT_QML_PLUGIN(TraceUITerminalPlugin)
#endif

int main(int argc, char *argv[]) {
    QQuickWindow::setDefaultAlphaBuffer(true);

    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);

#ifdef Q_OS_MACOS
    SystemMonitorMacOS systemMonitor;
    ProcessWatcherMacOS processWatcher;
    NetworkMonitorMacOS networkMonitor;

    qmlRegisterSingletonInstance<SystemMonitor>("TraceUI", 0, 1, "SystemMonitor", &systemMonitor);
    qmlRegisterSingletonInstance<ProcessWatcher>("TraceUI", 0, 1, "ProcessWatcher",
                                                 &processWatcher);
    qmlRegisterSingletonInstance<NetworkMonitor>("TraceUI", 0, 1, "NetworkMonitor",
                                                 &networkMonitor);
#endif

    qmlRegisterType<NetworkHistoryProvider>("TraceUI", 0, 1, "NetworkHistoryProvider");
    qmlRegisterType<CpuHistoryProvider>("TraceUI", 0, 1, "CpuHistoryProvider");

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/qt/qml"));
    const bool buildTerminal = BUILD_TERMINAL != 0;
    engine.rootContext()->setContextProperty(QStringLiteral("buildTerminal"), buildTerminal);
    engine.loadFromModule(APP_MODULE_NAME, "Main");

    return QGuiApplication::exec();
}
