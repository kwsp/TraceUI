#pragma once
#include <QSGMaterial>
#include <QSGTexture>

class EmojiMaterial : public QSGMaterial {
public:
    EmojiMaterial();
    ~EmojiMaterial() override = default;

    EmojiMaterial(const EmojiMaterial &) = delete;
    EmojiMaterial(EmojiMaterial &&) = delete;
    EmojiMaterial &operator=(const EmojiMaterial &) = delete;
    EmojiMaterial &operator=(EmojiMaterial &&) = delete;

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode mode) const override;
    int compare(const QSGMaterial *other) const override;

    void setTexture(QSGTexture *t) { m_texture = t; }
    QSGTexture *texture() const { return m_texture; }

private:
    QSGTexture *m_texture{};
};
