#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float graphScale;
    float phase;
};

layout(binding = 1) uniform sampler2D cpuHistory;

// Distance from point p to line segment a-b
float distToSegment(vec2 p, vec2 a, vec2 b) {
    vec2 ab = b - a;
    float t = clamp(dot(p - a, ab) / dot(ab, ab), 0.0, 1.0);
    return length(p - (a + t * ab));
}

void main() {
    vec2 uv = qt_TexCoord0;

    // Background grid: 8 cells
    float gy = uv.y * 8.0;
    float distY = abs(gy - round(gy)) / 8.0;
    float hLine = smoothstep(0.003, 0.0, distY);

    float gx = (uv.x + phase) * 8.0;
    float distX = abs(gx - round(gx)) / 8.0;
    float vLine = smoothstep(0.003, 0.0, distX);

    float gridLine = max(hLine, vLine);
    vec4 gridCol = vec4(0.15, 0.15, 0.15, 0.4) * gridLine;

    // CPU history: R = user, G = system (both normalized 0-1 representing 0-100%)
    float texelPos = uv.x * 128.0;
    float texelFloor = floor(texelPos);
    vec2 hist1 = texture(cpuHistory, vec2((texelFloor + 0.5) / 128.0, 0.5)).rg;
    vec2 hist2 = texture(cpuHistory, vec2((texelFloor + 1.5) / 128.0, 0.5)).rg;

    float x1 = (texelFloor + 0.5) / 128.0;
    float x2 = (texelFloor + 1.5) / 128.0;

    // User CPU curve: Gold (#c9a84c) — grows upward from bottom
    float userY1 = 1.0 - hist1.r * graphScale * 0.95;
    float userY2 = 1.0 - hist2.r * graphScale * 0.95;
    float distUser = distToSegment(uv, vec2(x1, userY1), vec2(x2, userY2));
    float beamUser = smoothstep(0.008, 0.0, distUser);
    float userAvg = mix(hist1.r, hist2.r, 0.5);
    float glowUser = 0.002 / max(distUser, 0.001) * smoothstep(0.0, 0.02, userAvg);
    float userYInterp = mix(userY1, userY2, clamp((uv.x - x1) / (x2 - x1), 0.0, 1.0));
    float userFill = step(userYInterp, uv.y);
    vec4 userCol = clamp(vec4(0.79, 0.66, 0.30, 1.0) * min(beamUser + glowUser, 1.0)
                        + vec4(0.35, 0.30, 0.15, 0.25) * userFill, 0.0, 1.0);

    // System CPU curve: Silver (#a8b0b8) — grows upward from bottom
    float sysY1 = 1.0 - hist1.g * graphScale * 0.95;
    float sysY2 = 1.0 - hist2.g * graphScale * 0.95;
    float distSys = distToSegment(uv, vec2(x1, sysY1), vec2(x2, sysY2));
    float beamSys = smoothstep(0.008, 0.0, distSys);
    float sysAvg = mix(hist1.g, hist2.g, 0.5);
    float glowSys = 0.002 / max(distSys, 0.001) * smoothstep(0.0, 0.02, sysAvg);
    float sysYInterp = mix(sysY1, sysY2, clamp((uv.x - x1) / (x2 - x1), 0.0, 1.0));
    float sysFill = step(sysYInterp, uv.y);
    vec4 sysCol = clamp(vec4(0.66, 0.69, 0.72, 1.0) * min(beamSys + glowSys, 1.0)
                        + vec4(0.30, 0.30, 0.30, 0.20) * sysFill, 0.0, 1.0);

    // Composite
    vec4 finalCol = gridCol;
    finalCol = mix(finalCol, sysCol, sysCol.a);
    finalCol = mix(finalCol, userCol, userCol.a);

    fragColor = finalCol * qt_Opacity;
}
