#version 400

layout(location=0) in vec4 in_position;

void main(void)
{
    gl_Position = in_position;
}
