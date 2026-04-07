#version 440

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 vFgColor;
layout(location = 2) in vec4 vBgColor;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};

layout(binding = 1) uniform sampler2D glyphAtlas;

void main() {
    vec4 texel = texture(glyphAtlas, vTexCoord);
    float alpha = texel.a;
    // Composite: glyph foreground over cell background
    vec3 color = mix(vBgColor.rgb, vFgColor.rgb, alpha);
    fragColor = vec4(color, 1.0) * qt_Opacity;
}
