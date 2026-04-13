#include "EmojiAtlas.h"
#include "EmojiMaterial.h"
#include "GlyphMaterial.h"
#include "TerminalBackend.h"
#include "TerminalRenderer.h"
#include <QFontDatabase>
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

class TestBundledFonts : public QObject {
    Q_OBJECT
private slots:
    void testBundledFontsLoad() {
        // Mirror what production main.cpp does — fonts must be loadable from Qt resources.
        const int hackId = QFontDatabase::addApplicationFont(
            QStringLiteral(":/fonts/Hack-Regular.ttf"));
        const int nfId = QFontDatabase::addApplicationFont(
            QStringLiteral(":/fonts/NF/SymbolsNerdFontMono-Regular.ttf"));
        QVERIFY(hackId != -1);
        QVERIFY(nfId   != -1);
        QVERIFY(QFontDatabase::families().contains(QStringLiteral("Hack")));
        QVERIFY(QFontDatabase::families().contains(QStringLiteral("Symbols Nerd Font Mono")));
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

class TestEmojiAtlas : public QObject {
    Q_OBJECT
private slots:
    void isEmojiDetectsMainBlocks() {
        QVERIFY(isEmoji(0x1F600));  // 😀 grinning face
        QVERIFY(isEmoji(0x1F525));  // 🔥 fire
        QVERIFY(isEmoji(0x2614));   // ☔ umbrella with rain (misc symbols)
        QVERIFY(isEmoji(0x23F0));   // ⏰ alarm clock (misc technical)
    }

    void isEmojiRejectsTextGlyphs() {
        QVERIFY(!isEmoji(0x0000));  // null
        QVERIFY(!isEmoji(0x0041));  // 'A'
        QVERIFY(!isEmoji(0x2500));  // box drawing ─
        QVERIFY(!isEmoji(0x2580));  // block element ▀
        QVERIFY(!isEmoji(0x00FF));  // Latin-1
    }

    void isEmojiRejectsPUA() {
        QVERIFY(!isEmoji(0xE000));  // Basic PUA start
        QVERIFY(!isEmoji(0xE348));  // Nerd Font Seti-UI
        QVERIFY(!isEmoji(0xF8FF));  // Basic PUA end
        QVERIFY(!isEmoji(0xF0000)); // SPUA-A start
        QVERIFY(!isEmoji(0xF04B2)); // Nerd Fonts v3 icon
        QVERIFY(!isEmoji(0xFFFFF)); // SPUA-A end
    }

    void isEmojiRejectsLibvtermContinuationSentinel() {
        // libvterm stamps continuation cells of double-wide glyphs with chars[0] = UINT32_MAX.
        // This value happens to be >= 0x1F000, so it would pass the emoji range check
        // without an explicit guard.
        QVERIFY(!isEmoji(static_cast<uint32_t>(-1)));
    }

    void ensureGlyphReturnsTrueOnMiss() {
        EmojiAtlas atlas;
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        QVERIFY(atlas.ensureGlyph(0x1F600) == true);
    }

    void ensureGlyphReturnsFalseOnHit() {
        EmojiAtlas atlas;
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        atlas.ensureGlyph(0x1F600);
        QVERIFY(atlas.ensureGlyph(0x1F600) == false);
    }

    void isDirtyAfterEnsureGlyph() {
        EmojiAtlas atlas;
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        atlas.markClean();
        atlas.ensureGlyph(0x1F600);
        QVERIFY(atlas.isDirty());
    }

    void markCleanClearsDirtyFlag() {
        EmojiAtlas atlas;
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        atlas.ensureGlyph(0x1F600);
        atlas.markClean();
        QVERIFY(!atlas.isDirty());
    }

    void uvHasValidCoordinatesAfterEnsure() {
        EmojiAtlas atlas;
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        atlas.ensureGlyph(0x1F600);
        GlyphUV uv = atlas.uv(0x1F600);
        QVERIFY(uv.u1 >= 0.0F && uv.u1 < uv.u2);
        QVERIFY(uv.v1 >= 0.0F && uv.v1 < uv.v2);
        QVERIFY(uv.u2 <= 1.0F);
        QVERIFY(uv.v2 <= 1.0F);
    }

    void setCellSizeInvalidatesCacheWhenDimensionsChange() {
        EmojiAtlas atlas;
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        atlas.ensureGlyph(0x1F600);
        atlas.markClean();
        // Different cell size — should reset
        atlas.setCellSize(10.0, 20.0, 1.0, 14.0);
        QVERIFY(atlas.isDirty());
        // Cache cleared — ensureGlyph should return true again
        QVERIFY(atlas.ensureGlyph(0x1F600) == true);
    }

    void setCellSizeIsNoopWhenDimensionsUnchanged() {
        EmojiAtlas atlas;
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        atlas.ensureGlyph(0x1F600);
        atlas.markClean();
        // Same dimensions — no-op
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        QVERIFY(!atlas.isDirty());
        // Cache intact
        QVERIFY(atlas.ensureGlyph(0x1F600) == false);
    }

    void dprChangeInvalidatesCache() {
        // A change in device pixel ratio requires a new physical atlas image.
        EmojiAtlas atlas;
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        atlas.ensureGlyph(0x1F600);
        atlas.markClean();
        // Different DPR — must reset
        atlas.setCellSize(8.0, 16.0, 2.0, 11.0);
        QVERIFY(atlas.isDirty());
        // Cache cleared — ensureGlyph should return true again
        QVERIFY(atlas.ensureGlyph(0x1F600) == true);
    }

    void emojiHeightChangeInvalidatesCache() {
        // A change in emojiHeight changes the rendered glyph size, so the atlas must reset.
        EmojiAtlas atlas;
        atlas.setCellSize(8.0, 16.0, 1.0, 11.0);
        atlas.ensureGlyph(0x1F600);
        atlas.markClean();
        // Different emojiHeight — must reset
        atlas.setCellSize(8.0, 16.0, 1.0, 14.0);
        QVERIFY(atlas.isDirty());
        QVERIFY(atlas.ensureGlyph(0x1F600) == true);
    }

    // Design-intent documentation test: isEmoji() is intentionally inclusive for the
    // 0x2600-0x27BF Misc Symbols range, which includes glyphs that terminals may render
    // as single-width (cell.width == 1, no VS16 selector). The libvterm sentinel guard in
    // TerminalRenderer is what prevents incorrectly rendering the continuation cell.
    void isEmojiIncludesMiscSymbols() {
        QVERIFY(isEmoji(0x2600)); // ☀ SUN — may be single-width without VS16
        QVERIFY(isEmoji(0x2603)); // ☃ SNOWMAN — may be single-width without VS16
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
    runTest(new TestBundledFonts);
    runTest(new TestEmojiMaterial);
    runTest(new TestEmojiAtlas);
    return status;
}

#include "test_TerminalRenderer.moc"
