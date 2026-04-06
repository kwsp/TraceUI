#include <QtGlobal>
#ifndef Q_OS_MACOS
int main(int, char *[]) {
    return 0;
}
#else
#include "../src/backend/macos/NetworkMonitorMacOS.h"
#include "../src/backend/macos/ProcessWatcherMacOS.h"
#include "../src/backend/macos/SystemMonitorMacOS.h"
#include <QQmlContext>
#include <QQmlEngine>
#include <QtQuickTest>

class Setup : public QObject {
    Q_OBJECT
public:
    Setup() {}

public slots:
    void qmlEngineAvailable(QQmlEngine *engine) {
        engine->rootContext()->setContextProperty("systemMonitor", new SystemMonitorMacOS(this));
        engine->rootContext()->setContextProperty("processWatcher", new ProcessWatcherMacOS(this));
        engine->rootContext()->setContextProperty("networkMonitor", new NetworkMonitorMacOS(this));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(UI, Setup)
#include "test_ui.moc"
#endif
