#include "EmojiMaterial.h"
#include <QSGMaterialShader>

// NOLINTBEGIN(*-isolate-declaration, *-static-cast-downcast)

class EmojiMaterialShader : public QSGMaterialShader {
public:
    EmojiMaterialShader() {
        setShaderFileName(VertexStage, QStringLiteral(":/terminal/shaders/emoji.vert.qsb"));
        setShaderFileName(FragmentStage, QStringLiteral(":/terminal/shaders/emoji.frag.qsb"));
    }

    bool updateUniformData(RenderState &state, QSGMaterial *newMat, QSGMaterial *oldMat) override {
        QByteArray *buf = state.uniformData();
        Q_ASSERT(buf->size() >= 68);
        bool changed = false;
        if ((oldMat == nullptr) || state.isMatrixDirty()) {
            const QMatrix4x4 m = state.combinedMatrix();
            memcpy(buf->data(), m.constData(), 64);
            changed = true;
        }
        if ((oldMat == nullptr) || state.isOpacityDirty()) {
            float opacity = state.opacity();
            memcpy(buf->data() + 64, &opacity, 4);
            changed = true;
        }
        return changed;
    }

    void updateSampledImage(RenderState &state, int binding, QSGTexture **texture,
                            QSGMaterial *newMat, QSGMaterial *_) override {
        if (binding == 1) {
            auto *mat = static_cast<EmojiMaterial *>(newMat);
            if (mat->texture()) {
                mat->texture()->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
                *texture = mat->texture();
            }
        }
    }
};

EmojiMaterial::EmojiMaterial() {
    setFlag(Blending, true);
    setFlag(RequiresFullMatrix, true);
}

QSGMaterialType *EmojiMaterial::type() const {
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *EmojiMaterial::createShader(QSGRendererInterface::RenderMode) const {
    return new EmojiMaterialShader;
}

int EmojiMaterial::compare(const QSGMaterial *other) const {
    auto *m = static_cast<const EmojiMaterial *>(other);
    if (m_texture == m->texture())
        return 0;
    return m_texture < m->texture() ? -1 : 1;
}

// NOLINTEND(*-isolate-declaration, *-static-cast-downcast)