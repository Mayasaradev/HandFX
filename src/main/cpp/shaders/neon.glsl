#version 300 es
precision mediump float;
in vec2 v_uv;
out vec4 fragColor;

uniform sampler2D u_yTex;
uniform sampler2D u_uvTex;
uniform vec2 u_hand; // dari hand_tracker 0-1
uniform float u_glow; // dari slider ImGui

void main(){
    // SHADER 1: YUV -> RGB
    float y = texture(u_yTex, v_uv).r;
    // SHADER 2: Glow distance
    float d = distance(v_uv, u_hand);
    float glow = exp(-d * 25.0) * u_glow;
    // SHADER 3: Composite
    vec3 cam = vec3(y); // grayscale dulu biar kenceng
    vec3 neon = vec3(0.0, glow, 1.0) * glow;
    fragColor = vec4(cam + neon, 1.0);
}
