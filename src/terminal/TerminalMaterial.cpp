#include "TerminalMaterial.h"
#include <QSGTexture>

// ── Attribute layout ─────────────────────────────────────────────────────────

static const QSGGeometry::Attribute kTerminalAttributes[] = {
    QSGGeometry::Attribute::createWithAttributeType(0, 2, QSGGeometry::FloatType,
                                                    QSGGeometry::PositionAttribute),
    QSGGeometry::Attribute::createWithAttributeType(1, 2, QSGGeometry::FloatType,
                                                    QSGGeometry::TexCoordAttribute),
    QSGGeometry::Attribute::createWithAttributeType(2, 4, QSGGeometry::FloatType,
                                                    QSGGeometry::UnknownAttribute),
    QSGGeometry::Attribute::createWithAttributeType(3, 4, QSGGeometry::FloatType,
                                                    QSGGeometry::UnknownAttribute),
};

static QSGGeometry::AttributeSet kTerminalAttributeSet = {
    4,                      // attribute count
    sizeof(TerminalVertex), // stride
    kTerminalAttributes,
};

QSGGeometry::AttributeSet &terminalAttributeSet() {
    return kTerminalAttributeSet;
}

// ── TerminalMaterialShader ───────────────────────────────────────────────────

TerminalMaterialShader::TerminalMaterialShader() {
    setShaderFileName(VertexStage, QStringLiteral(":/terminal/shaders/terminal.vert.qsb"));
    setShaderFileName(FragmentStage, QStringLiteral(":/terminal/shaders/terminal.frag.qsb"));
}

bool TerminalMaterialShader::updateUniformData(RenderState &state, QSGMaterial * /*newMat*/,
                                               QSGMaterial * /*oldMat*/) {
    bool changed = false;
    QByteArray *buf = state.uniformData();

    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data(), m.constData(), 64);
        changed = true;
    }

    if (state.isOpacityDirty()) {
        float opacity = state.opacity();
        memcpy(buf->data() + 64, &opacity, 4);
        changed = true;
    }

    return changed;
}

void TerminalMaterialShader::updateSampledImage(RenderState &state, int binding,
                                                QSGTexture **texture, QSGMaterial *newMat,
                                                QSGMaterial * /*oldMat*/) {
    if (binding == 1) {
        auto *mat = static_cast<TerminalMaterial *>(newMat);
        if (mat->texture()) {
            mat->texture()->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
            *texture = mat->texture();
        }
    }
}

// ── TerminalMaterial ─────────────────────────────────────────────────────────

TerminalMaterial::TerminalMaterial() {
    setFlag(Blending);
}

QSGMaterialType *TerminalMaterial::type() const {
    static QSGMaterialType t;
    return &t;
}

QSGMaterialShader *TerminalMaterial::createShader(QSGRendererInterface::RenderMode) const {
    return new TerminalMaterialShader;
}

int TerminalMaterial::compare(const QSGMaterial *other) const {
    auto *o = static_cast<const TerminalMaterial *>(other);
    if (m_texture != o->m_texture)
        return m_texture < o->m_texture ? -1 : 1;
    return 0;
}
