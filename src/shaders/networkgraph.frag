#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float time;
};

layout(binding = 1) uniform sampler2D networkHistory;

void main() {
    vec2 uv = qt_TexCoord0;

    // Scrolling background grid
    float moveSpeed = 0.1;
    float timeShift = time * moveSpeed;
    vec2 gridUV = fract(vec2((uv.x + timeShift) * 20.0, uv.y * 10.0));
    float gridLine = step(gridUV.x, 0.05) + step(gridUV.y, 0.05);
    vec3 gridCol = vec3(0.1, 0.1, 0.1) * min(gridLine, 1.0);

    // Center divider
    float centerLine = smoothstep(0.005, 0.0, abs(uv.y - 0.5));
    gridCol += vec3(0.15, 0.15, 0.15) * centerLine;

    // Sample history: R = download, G = upload
    vec2 hist = texture(networkHistory, vec2(uv.x, 0.5)).rg;
    float dlVal = hist.r;
    float ulVal = hist.g;

    // DL curve: grows upward from center (0.5 → 0.95)
    float dlY = 0.5 + dlVal * 0.45;
    float distDL = abs(uv.y - dlY);
    float beamDL = smoothstep(0.01, 0.0, distDL);
    float glowDL = 0.003 / max(distDL, 0.001);
    float dlFill = step(0.5, uv.y) * step(uv.y, dlY);
    vec3 dlCol = vec3(0.0, 0.9, 1.0) * (beamDL + glowDL) + vec3(0.0, 0.3, 0.4) * dlFill * 0.25;

    // UL curve: grows downward from center (0.5 → 0.05)
    float ulY = 0.5 - ulVal * 0.45;
    float distUL = abs(uv.y - ulY);
    float beamUL = smoothstep(0.01, 0.0, distUL);
    float glowUL = 0.003 / max(distUL, 0.001);
    float ulFill = step(ulY, uv.y) * step(uv.y, 0.5);
    vec3 ulCol = vec3(1.0, 0.5, 0.0) * (beamUL + glowUL) + vec3(0.4, 0.2, 0.0) * ulFill * 0.25;

    vec3 finalCol = gridCol + dlCol + ulCol;
    fragColor = vec4(finalCol, 1.0) * qt_Opacity;
}
