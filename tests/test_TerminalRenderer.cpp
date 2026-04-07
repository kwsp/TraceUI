#include "TerminalBackend.h"
#include "TerminalMaterial.h"
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
        QVERIFY(renderer.cellHeight() > renderer.cellWidth());
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

        renderer.setBackend(&backend);
        QCOMPARE(spy.count(), 1);

        renderer.setBackend(nullptr);
        QVERIFY(renderer.backend() == nullptr);
        QCOMPARE(spy.count(), 2);
    }

    void testFontSizeClamped() {
        TerminalRenderer renderer;
        QSignalSpy spy(&renderer, &TerminalRenderer::fontSizeChanged);

        renderer.setFontSize(0);
        QCOMPARE(spy.count(), 0);
        QCOMPARE(renderer.fontSize(), 14);

        renderer.setFontSize(-5);
        QCOMPARE(spy.count(), 0);
    }

    void testVertexLayout() {
        const auto &attrs = terminalAttributeSet();
        QCOMPARE(attrs.stride, static_cast<int>(sizeof(TerminalVertex)));
        QCOMPARE(attrs.count, 4); // pos, texCoord, fgColor, bgColor
    }

    void testVertexSet() {
        TerminalVertex v;
        v.set(10, 20, 0.5F, 0.5F, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 1.0F);
        QCOMPARE(v.x, 10.0F);
        QCOMPARE(v.y, 20.0F);
        QCOMPARE(v.u, 0.5F);
        QCOMPARE(v.v, 0.5F);
        QCOMPARE(v.fgR, 1.0F);
        QCOMPARE(v.bgR, 0.0F);
        QCOMPARE(v.bgA, 1.0F);
    }

    void testMaterialType() {
        TerminalMaterial mat1;
        TerminalMaterial mat2;
        QCOMPARE(mat1.type(), mat2.type());
    }
};

QTEST_MAIN(TestTerminalRenderer)
#include "test_TerminalRenderer.moc"
