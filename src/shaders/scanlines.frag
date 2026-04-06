#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
} ubuf;

void main() {
    // 1080 is a reference height, adjust for density if needed
    float line = step(0.5, mod(qt_TexCoord0.y * 1080.0, 2.0));
    fragColor = vec4(0.0, 0.0, 0.0, line) * ubuf.qt_Opacity;
}
