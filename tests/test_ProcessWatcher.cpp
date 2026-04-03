#include <QtGlobal>
#ifndef Q_OS_MACOS
int main(int, char*[]) { return 0; }
#else
#include <QtTest>
#include <QCoreApplication>
#include "backend/macos/ProcessWatcherMacOS.h"
#include "backend/ProcessListModel.h"

class ProcessWatcherTest : public QObject {
    Q_OBJECT

private slots:
    void testInitialValues() {
        ProcessWatcherMacOS watcher;
        QVERIFY(watcher.processes()->rowCount({}) == 0);
    }

    void testUpdateProvidesProcesses() {
        ProcessWatcherMacOS watcher;
        watcher.update();

        ProcessListModel* model = watcher.processes();
        QVERIFY(model->rowCount({}) > 0);

        bool foundKnownProcess = false;
        for (int i = 0; i < model->rowCount({}); ++i) {
            const QModelIndex idx = model->index(i);
            QVERIFY(model->data(idx, ProcessListModel::PidRole).isValid());
            QVERIFY(model->data(idx, ProcessListModel::NameRole).isValid());
            QVERIFY(model->data(idx, ProcessListModel::RamMBRole).isValid());
            const QString name = model->data(idx, ProcessListModel::NameRole).toString();
            if (name == "test_ProcessWatcher" || name == "launchd")
                foundKnownProcess = true;
        }
        QVERIFY(foundKnownProcess);
    }

    void testResultsAreSortedByRam() {
        ProcessWatcherMacOS watcher;
        watcher.update();

        ProcessListModel* model = watcher.processes();
        for (int i = 1; i < model->rowCount({}); ++i) {
            const int prev = model->data(model->index(i - 1), ProcessListModel::RamMBRole).toInt();
            const int curr = model->data(model->index(i),     ProcessListModel::RamMBRole).toInt();
            QVERIFY(prev >= curr);
        }
    }
};

QTEST_MAIN(ProcessWatcherTest)
#include "test_ProcessWatcher.moc"
#endif
