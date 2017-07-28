#version 400

layout(location=0) in vec4 in_position;

uniform mat4 transformMatrix;

void main(void)
{
    gl_Position = transformMatrix * in_position;
}
