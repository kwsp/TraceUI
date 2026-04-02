#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 resolution;
    float spacing;
    float radius;
    float dotAlpha;
};

void main() {
    vec2 pos = qt_TexCoord0 * resolution;
    vec2 nearest = round(pos / spacing) * spacing;
    float dist = length(pos - nearest);
    float mask = 1.0 - smoothstep(radius - 0.5, radius + 0.5, dist);
    float a = mask * dotAlpha * qt_Opacity;
    fragColor = vec4(a, a, a, a);  // white, premultiplied
}
