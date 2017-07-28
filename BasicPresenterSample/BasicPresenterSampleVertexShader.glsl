#version 400

layout(location=0) in vec4 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_textureCoordinate;

out vec3 inNormal;
out vec2 textureCoordinate;
out vec4 eyePosition;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

void main(void)
{
  mat3 normalMatrix = mat3(modelViewMatrix);
  gl_Position = projectionMatrix * modelViewMatrix * in_position;
  eyePosition = modelViewMatrix * in_position;
  inNormal = normalMatrix * in_normal;
  textureCoordinate = in_textureCoordinate;
}