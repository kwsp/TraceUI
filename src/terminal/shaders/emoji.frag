#version 440

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 vColor;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};

layout(binding = 1) uniform sampler2D emojiAtlas;

void main() {
    fragColor = texture(emojiAtlas, vTexCoord) * qt_Opacity;
}
