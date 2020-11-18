#version 330 core

// The position variable has attribute #0
layout (location = 0) in vec3 position;

// The color variable has attribute #1
layout (location = 1) in vec3 color;

// Specify a color output to the fragment shader
out vec3 vertex_color;

void main()
{
    gl_Position = vec4(position, 1.0);
    vertex_color = color;
}
