#include "EmojiMaterial.h"
#include "GlyphMaterial.h"
#include "TerminalBackend.h"
#include "TerminalRenderer.h"
#include <QGuiApplication>
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

    void testCellMetricsAreIntegers() {
        TerminalRenderer renderer;
        renderer.ensureMetrics();
        // Cell dimensions should be snapped to integers (std::ceil)
        QCOMPARE(renderer.cellWidth(), std::ceil(renderer.cellWidth()));
        QCOMPARE(renderer.cellHeight(), std::ceil(renderer.cellHeight()));
    }
};

class TestEmojiMaterial : public QObject {
    Q_OBJECT
private slots:
    void typeIsDifferentFromGlyphMaterial() {
        EmojiMaterial emoji;
        GlyphMaterial glyph;
        QVERIFY(emoji.type() != glyph.type());
    }

    void hasBlendingFlag() {
        EmojiMaterial mat;
        QVERIFY(mat.flags() & QSGMaterial::Blending);
    }
};

// Run multiple test objects in one binary
int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    int status = 0;
    auto runTest = [&](QObject *obj) {
        status |= QTest::qExec(obj, argc, argv);
        delete obj;
    };
    runTest(new TestTerminalRenderer);
    runTest(new TestEmojiMaterial);
    return status;
}

#include "test_TerminalRenderer.moc"
