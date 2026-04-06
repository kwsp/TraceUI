#include <QSignalSpy>
#include <QTest>
#include "TerminalRenderer.h"
#include "TerminalBackend.h"

class TestTerminalRenderer : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Any global init
    }

    void testProperties() {
        TerminalRenderer renderer;
        QSignalSpy backendSpy(&renderer, &TerminalRenderer::backendChanged);
        QSignalSpy fontSpy(&renderer, &TerminalRenderer::fontDataChanged);
        QSignalSpy sizeSpy(&renderer, &TerminalRenderer::fontSizeChanged);

        renderer.setFontData("Courier");
        QCOMPARE(renderer.fontData(), QString("Courier"));
        QCOMPARE(fontSpy.count(), 1);

        renderer.setFontSize(14);
        QCOMPARE(renderer.fontSize(), 14);
        QCOMPARE(sizeSpy.count(), 1);

        TerminalBackend backend;
        renderer.setBackend(&backend);
        QCOMPARE(renderer.backend(), &backend);
        QCOMPARE(backendSpy.count(), 1);
    }

    void testMetrics() {
        TerminalRenderer renderer;
        QSignalSpy metricsSpy(&renderer, &TerminalRenderer::cellMetricsChanged);

        // Accessing properties should trigger updateCellMetrics if dirty
        qreal w = renderer.cellWidth();
        qreal h = renderer.cellHeight();
        
        // Initial metrics are calculated upon first access or update
        QVERIFY(w > 0);
        QVERIFY(h > 0);
        QCOMPARE(metricsSpy.count(), 1);

        renderer.setFontSize(24);
        QVERIFY(renderer.cellHeight() > h);
        QCOMPARE(metricsSpy.count(), 2);
    }
};

QTEST_MAIN(TestTerminalRenderer)
#include "test_TerminalRenderer.moc"
