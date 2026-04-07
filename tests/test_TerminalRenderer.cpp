#include "TerminalBackend.h"
#include "TerminalRenderer.h"
#include <QSignalSpy>
#include <QTest>

class TestTerminalRenderer : public QObject {
    Q_OBJECT

private slots:
    void testDefaultMetrics() {
        TerminalRenderer renderer;
        renderer.ensureMetrics();

        QVERIFY(renderer.cellWidth() > 0);
        QVERIFY(renderer.cellHeight() > 0);
        QVERIFY(renderer.cellHeight() > renderer.cellWidth()); // monospace cells are taller
    }

    void testFontSizeAffectsMetrics() {
        TerminalRenderer renderer;
        renderer.ensureMetrics();

        qreal h14 = renderer.cellHeight();

        QSignalSpy metricsSpy(&renderer, &TerminalRenderer::cellMetricsChanged);
        renderer.setFontSize(24);
        QCOMPARE(metricsSpy.count(), 1);
        QVERIFY(renderer.cellHeight() > h14);
    }

    void testFontFamilySignal() {
        TerminalRenderer renderer;
        QSignalSpy spy(&renderer, &TerminalRenderer::fontFamilyChanged);

        renderer.setFontFamily("Courier");
        QCOMPARE(renderer.fontFamily(), QString("Courier"));
        QCOMPARE(spy.count(), 1);

        // No-op if same value
        renderer.setFontFamily("Courier");
        QCOMPARE(spy.count(), 1);
    }

    void testBackendProperty() {
        TerminalRenderer renderer;
        QSignalSpy spy(&renderer, &TerminalRenderer::backendChanged);

        TerminalBackend backend;
        renderer.setBackend(&backend);
        QCOMPARE(renderer.backend(), &backend);
        QCOMPARE(spy.count(), 1);

        // No-op
        renderer.setBackend(&backend);
        QCOMPARE(spy.count(), 1);

        // Clear
        renderer.setBackend(nullptr);
        QVERIFY(renderer.backend() == nullptr);
        QCOMPARE(spy.count(), 2);
    }

    void testFontSizeClamped() {
        TerminalRenderer renderer;
        QSignalSpy spy(&renderer, &TerminalRenderer::fontSizeChanged);

        renderer.setFontSize(0);           // invalid
        QCOMPARE(spy.count(), 0);          // should be rejected
        QCOMPARE(renderer.fontSize(), 14); // unchanged from default

        renderer.setFontSize(-5); // invalid
        QCOMPARE(spy.count(), 0);
    }
};

QTEST_MAIN(TestTerminalRenderer)
#include "test_TerminalRenderer.moc"
