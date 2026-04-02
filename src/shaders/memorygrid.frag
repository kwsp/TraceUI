#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float usageRatio;
};

void main() {
    vec2 uv = qt_TexCoord0;
    
    // Gold blocks
    vec3 col = vec3(0.788, 0.658, 0.298); 
    
    // Grid cells
    vec2 gridUV = fract(uv * vec2(20.0, 5.0));
    ivec2 gridIndex = ivec2(floor(uv * vec2(20.0, 5.0)));
    
    // 1D index
    float linearIndex = float(gridIndex.x + gridIndex.y * 20) / 100.0;
    
    float activeBlock = (linearIndex < usageRatio) ? 1.0 : 0.2;
    
    // Borders
    float border = smoothstep(0.0, 0.1, gridUV.x) * smoothstep(1.0, 0.9, gridUV.x) *
                   smoothstep(0.0, 0.1, gridUV.y) * smoothstep(1.0, 0.9, gridUV.y);
                   
    fragColor = vec4(col * activeBlock * border, 1.0) * qt_Opacity;
}
