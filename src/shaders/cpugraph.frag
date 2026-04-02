#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float currentUsage;
    float time;
};

void main() {
    vec2 uv = qt_TexCoord0;
    
    // Neo-Kitsch Gold
    vec3 col = vec3(0.788, 0.658, 0.298); 
    
    // Procedural wave simulating an ECG/CPU graph using currentUsage as amplitude
    float wave = sin(uv.x * 10.0 - time * 2.0) * cos(uv.x * 20.0 + time) * 0.2;
    wave *= currentUsage;
    
    float baseline = 0.5 + wave;
    
    // Line thickness
    float thickness = 0.02;
    float dist = abs(uv.y - baseline);
    
    float alpha = smoothstep(thickness, thickness * 0.5, dist);
    
    // Fill under line
    if (uv.y > baseline) {
        alpha += 0.2 * currentUsage * smoothstep(1.0, baseline, uv.y);
    }
    
    fragColor = vec4(col, 1.0) * alpha * qt_Opacity;
}
