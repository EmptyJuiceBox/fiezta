#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex {
  vec4 gl_Position;
};

layout(row_major, push_constant) uniform Constants {
  mat4 mvp;
};

void main() {
  gl_Position = mvp * vec4(position, 1.0);
  fragColor = color;
  fragTexCoord = texCoord;
}
