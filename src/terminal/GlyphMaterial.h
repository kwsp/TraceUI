#pragma once
#include <QSGMaterial>
#include <QSGTexture>

struct GlyphVertex {
    float x, y;
    float u, v;
    float r, g, b, a;
    void set(float _x, float _y, float _u, float _v, float _r, float _g, float _b, float _a) {
        x = _x;
        y = _y;
        u = _u;
        v = _v;
        r = _r;
        g = _g;
        b = _b;
        a = _a;
    }
};

struct GlyphUV {
    float u1, v1, u2, v2;
};

class GlyphMaterial : public QSGMaterial {
public:
    GlyphMaterial();
    ~GlyphMaterial() override = default;

    GlyphMaterial(const GlyphMaterial &) = delete;
    GlyphMaterial(GlyphMaterial &&) = delete;
    GlyphMaterial &operator=(const GlyphMaterial &) = delete;
    GlyphMaterial &operator=(GlyphMaterial &&) = delete;

    [[nodiscard]] QSGMaterialType *type() const override;
    [[nodiscard]] QSGMaterialShader *
    createShader(QSGRendererInterface::RenderMode mode) const override;
    int compare(const QSGMaterial *other) const override;

    void setTexture(QSGTexture *texture) { m_texture = texture; }
    [[nodiscard]] QSGTexture *texture() const { return m_texture; }

private:
    QSGTexture *m_texture = nullptr;
};
