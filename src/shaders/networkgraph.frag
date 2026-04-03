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

// Distance from point p to line segment a-b
float distToSegment(vec2 p, vec2 a, vec2 b) {
    vec2 ab = b - a;
    float t = clamp(dot(p - a, ab) / dot(ab, ab), 0.0, 1.0);
    return length(p - (a + t * ab));
}

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
    // Sample adjacent texel centers for line segment rendering
    float texelPos = uv.x * 128.0;
    float texelFloor = floor(texelPos);
    vec2 hist1 = texture(networkHistory, vec2((texelFloor + 0.5) / 128.0, 0.5)).rg;
    vec2 hist2 = texture(networkHistory, vec2((texelFloor + 1.5) / 128.0, 0.5)).rg;

    // Endpoint x-coords in UV space
    float x1 = (texelFloor + 0.5) / 128.0;
    float x2 = (texelFloor + 1.5) / 128.0;

    // DL curve: Gold (#c9a84c) — line segment between adjacent points
    float dlY1 = 0.5 - hist1.r * graphScale * 0.45;
    float dlY2 = 0.5 - hist2.r * graphScale * 0.45;
    float distDL = distToSegment(uv, vec2(x1, dlY1), vec2(x2, dlY2));
    float beamDL = smoothstep(0.01, 0.0, distDL);
    float dlAvg = mix(hist1.r, hist2.r, 0.5);
    float glowDL = 0.003 / max(distDL, 0.001) * smoothstep(0.0, 0.02, dlAvg);
    float dlYInterp = mix(dlY1, dlY2, clamp((uv.x - x1) / (x2 - x1), 0.0, 1.0));
    float dlFill = step(dlYInterp, uv.y) * step(uv.y, 0.5);
    vec4 dlCol = vec4(0.79, 0.66, 0.30, 1.0) * min(beamDL + glowDL, 1.0) + vec4(0.35, 0.30, 0.15, 0.3) * dlFill;

    // UL curve: Silver (#a8b0b8) — line segment between adjacent points
    float ulY1 = 0.5 + hist1.g * graphScale * 0.45;
    float ulY2 = 0.5 + hist2.g * graphScale * 0.45;
    float distUL = distToSegment(uv, vec2(x1, ulY1), vec2(x2, ulY2));
    float beamUL = smoothstep(0.01, 0.0, distUL);
    float ulAvg = mix(hist1.g, hist2.g, 0.5);
    float glowUL = 0.003 / max(distUL, 0.001) * smoothstep(0.0, 0.02, ulAvg);
    float ulYInterp = mix(ulY1, ulY2, clamp((uv.x - x1) / (x2 - x1), 0.0, 1.0));
    float ulFill = step(0.5, uv.y) * step(uv.y, ulYInterp);
    vec4 ulCol = vec4(0.66, 0.69, 0.72, 1.0) * min(beamUL + glowUL, 1.0) + vec4(0.30, 0.30, 0.30, 0.3) * ulFill;

    vec4 finalCol = gridCol;
    finalCol = mix(finalCol, dlCol, dlCol.a);
    finalCol = mix(finalCol, ulCol, ulCol.a);

    fragColor = finalCol * qt_Opacity;
}
