#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float graphScale;
    float phase;
};

layout(binding = 1) uniform sampler2D networkHistory;

void main() {
    vec2 uv = qt_TexCoord0;

    // Scrolling background grid: transparent between lines
    vec2 gridUV = fract(vec2((uv.x + phase) * 20.0, uv.y * 10.0));
    float gridLine = step(gridUV.x, 0.05) + step(gridUV.y, 0.05);
    vec4 gridCol = vec4(0.15, 0.15, 0.15, 0.4) * step(0.5, gridLine);

    // Center divider
    float centerLine = smoothstep(0.005, 0.0, abs(uv.y - 0.5));
    gridCol += vec4(0.2, 0.2, 0.2, 0.6) * centerLine;

    // Network history: R = download, G = upload
    vec2 hist = texture(networkHistory, vec2(uv.x, 0.5)).rg;
    float dlVal = hist.r;
    float ulVal = hist.g;

    // DL curve: Gold (#c9a84c)
    float dlY = 0.5 - dlVal * graphScale * 0.45;
    float distDL = abs(uv.y - dlY);
    float beamDL = smoothstep(0.01, 0.0, distDL);
    float glowDL = 0.003 / max(distDL, 0.001);
    float dlFill = step(dlY, uv.y) * step(uv.y, 0.5);
    vec4 dlCol = vec4(0.79, 0.66, 0.30, 1.0) * (beamDL + glowDL) + vec4(0.35, 0.30, 0.15, 0.3) * dlFill;

    // UL curve: Silver (#a8b0b8)
    float ulY = 0.5 + ulVal * graphScale * 0.45;
    float distUL = abs(uv.y - ulY);
    float beamUL = smoothstep(0.01, 0.0, distUL);
    float glowUL = 0.003 / max(distUL, 0.001);
    float ulFill = step(0.5, uv.y) * step(uv.y, ulY);
    vec4 ulCol = vec4(0.66, 0.69, 0.72, 1.0) * (beamUL + glowUL) + vec4(0.30, 0.30, 0.30, 0.3) * ulFill;

    vec4 finalCol = gridCol;
    finalCol = mix(finalCol, dlCol, dlCol.a);
    finalCol = mix(finalCol, ulCol, ulCol.a);

    fragColor = finalCol * qt_Opacity;
}
