#include "GlyphMaterial.h"
#include <QSGMaterialShader>

class GlyphMaterialShader : public QSGMaterialShader {
public:
    GlyphMaterialShader() {
        setShaderFileName(VertexStage, QLatin1String(":/terminal/shaders/glyph.vert.qsb"));
        setShaderFileName(FragmentStage, QLatin1String(":/terminal/shaders/glyph.frag.qsb"));
    }

    bool updateUniformData(RenderState &state, QSGMaterial *newMat, QSGMaterial *oldMat) override {
        QByteArray *buf = state.uniformData();
        Q_ASSERT(buf->size() >= 68);
        bool changed = false;

        // If oldMat is null, this is the first time we use this uniform buffer.
        // We must initialize everything to prevent reading garbage (which caused the noise bug).
        if (!oldMat || state.isMatrixDirty()) {
            const QMatrix4x4 m = state.combinedMatrix();
            memcpy(buf->data(), m.constData(), 64);
            changed = true;
        }

        if (!oldMat || state.isOpacityDirty()) {
            float opacity = state.opacity();
            memcpy(buf->data() + 64, &opacity, 4);
            changed = true;
        }

        return changed;
    }

    void updateSampledImage(RenderState &state, int binding, QSGTexture **texture,
                            QSGMaterial *newMat, QSGMaterial *) override {
        if (binding == 1) {
            auto *mat = static_cast<GlyphMaterial *>(newMat);
            if (mat->texture()) {
                mat->texture()->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
                *texture = mat->texture();
            }
        }
    }
};

GlyphMaterial::GlyphMaterial() {
    setFlag(Blending, true);
    setFlag(RequiresFullMatrix, true);
}

QSGMaterialType *GlyphMaterial::type() const {
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *GlyphMaterial::createShader(QSGRendererInterface::RenderMode) const {
    return new GlyphMaterialShader;
}

int GlyphMaterial::compare(const QSGMaterial *other) const {
    auto *m = static_cast<const GlyphMaterial *>(other);
    if (m_texture == m->texture())
        return 0;
    return m_texture < m->texture() ? -1 : 1;
}
