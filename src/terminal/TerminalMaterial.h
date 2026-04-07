#pragma once

#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QSGTexture>

// Custom vertex layout: pos(2f) + texCoord(2f) + fgColor(4f) + bgColor(4f) = 48 bytes
struct TerminalVertex {
    float x, y;               // position
    float u, v;               // texture coordinate
    float fgR, fgG, fgB, fgA; // foreground color
    float bgR, bgG, bgB, bgA; // background color

    void set(float px, float py, float tu, float tv, float fr, float fg, float fb, float fa,
             float br, float bg, float bb, float ba) {
        x = px;
        y = py;
        u = tu;
        v = tv;
        fgR = fr;
        fgG = fg;
        fgB = fb;
        fgA = fa;
        bgR = br;
        bgG = bg;
        bgB = bb;
        bgA = ba;
    }
};

class TerminalMaterialShader : public QSGMaterialShader {
public:
    TerminalMaterialShader();
    bool updateUniformData(RenderState &state, QSGMaterial *newMaterial,
                           QSGMaterial *oldMaterial) override;
    void updateSampledImage(RenderState &state, int binding, QSGTexture **texture,
                            QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
};

class TerminalMaterial : public QSGMaterial {
public:
    TerminalMaterial();

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;
    int compare(const QSGMaterial *other) const override;

    QSGTexture *texture() const { return m_texture; }
    void setTexture(QSGTexture *tex) { m_texture = tex; }

private:
    QSGTexture *m_texture{};
};

QSGGeometry::AttributeSet &terminalAttributeSet();
