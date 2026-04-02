#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float dlSignal;
    float ulSignal;
    float time;
};

void main() {
    vec2 uv = qt_TexCoord0;
    
    // Slowly shift everything to the left
    float moveSpeed = 0.1; // 10% of window per second = 10 seconds to span screen
    float timeShift = time * moveSpeed;
    float shiftedX = uv.x + timeShift;
    
    // Draw background grid lines (20x10) synced with time
    vec2 gridUV = fract(vec2(shiftedX * 20.0, uv.y * 10.0));
    float gridLine = step(gridUV.x, 0.05) + step(gridUV.y, 0.05);
    vec3 gridCol = vec3(0.1, 0.1, 0.1) * min(gridLine, 1.0);

    // Download Curve (Cyan)
    float baseWaveDL = sin(shiftedX * 30.0);
    // Amplitude maps linearly to dlSignal
    float pulseDL = baseWaveDL * (dlSignal * 0.4 + 0.01); 
    float distDL = abs(uv.y - (0.7 + pulseDL)); 
    float beamDL = smoothstep(0.01, 0.0, distDL);  
    float glowDL = 0.003 / max(distDL, 0.001);     
    vec3 dlCol = vec3(0.0, 0.9, 1.0) * (beamDL + glowDL);
    
    // Upload Curve (Orange)
    float baseWaveUL = sin(shiftedX * 24.0);
    // Amplitude maps linearly to ulSignal
    float pulseUL = baseWaveUL * (ulSignal * 0.4 + 0.01); 
    float distUL = abs(uv.y - (0.3 + pulseUL)); 
    float beamUL = smoothstep(0.01, 0.0, distUL);  
    float glowUL = 0.003 / max(distUL, 0.001);     
    vec3 ulCol = vec3(1.0, 0.5, 0.0) * (beamUL + glowUL);
    
    vec3 finalCol = gridCol + dlCol + ulCol;
    
    fragColor = vec4(finalCol, 1.0) * qt_Opacity;
}
