#version 440

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 color;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};

void main() {
    vTexCoord = texcoord;
    vColor = vec4(color.rgb, color.a * qt_Opacity);
    gl_Position = qt_Matrix * vec4(pos, 0.0, 1.0);
}
