#version 440

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec4 fgColor;
layout(location = 3) in vec4 bgColor;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vFgColor;
layout(location = 2) out vec4 vBgColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};

void main() {
    gl_Position = qt_Matrix * vec4(pos, 0.0, 1.0);
    vTexCoord = texCoord;
    vFgColor = fgColor;
    vBgColor = bgColor;
}
