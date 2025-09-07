#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform float u_time;
uniform sampler2D u_texture;

void main()
{
    vec4 texColor = texture(u_texture, vTexCoord);
    FragColor = texColor;
}