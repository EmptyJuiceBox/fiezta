#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 fragColor;

layout(row_major, set = 0, binding = 0) uniform PerObject {
  mat4 model;
};

layout(row_major, push_constant) uniform Constants {
  mat4 viewProj;
};

void main() {
  gl_Position = viewProj * model * vec4(position, 1.0);
  fragColor = normal;
}
