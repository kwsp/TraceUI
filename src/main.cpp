#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtQml/qqml.h>

#include "backend/CpuHistoryProvider.h"
#include "backend/NetworkHistoryProvider.h"
#include "backend/TerminalBackend.h"
#include "backend/TerminalModel.h"

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

  qmlRegisterSingletonInstance<SystemMonitor>("TraceUI", 0, 1, "SystemMonitor",
                                              &systemMonitor);
  qmlRegisterSingletonInstance<ProcessWatcher>(
      "TraceUI", 0, 1, "ProcessWatcher", &processWatcher);
  qmlRegisterSingletonInstance<NetworkMonitor>(
      "TraceUI", 0, 1, "NetworkMonitor", &networkMonitor);
#endif

  qmlRegisterType<NetworkHistoryProvider>("TraceUI", 0, 1,
                                          "NetworkHistoryProvider");
  qmlRegisterType<CpuHistoryProvider>("TraceUI", 0, 1, "CpuHistoryProvider");
  qmlRegisterType<TerminalBackend>("TraceUI", 0, 1, "TerminalBackend");
  qmlRegisterType<TerminalModel>("TraceUI", 0, 1, "TerminalModel");

  QQmlApplicationEngine engine;
  engine.loadFromModule(APP_MODULE_NAME, "Main");

  return QGuiApplication::exec();
}
