#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
  float tex = texture(texSampler, fragTexCoord).r;
  outColor = vec4(fragColor, 1.0) * tex;
}
