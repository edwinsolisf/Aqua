#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec2 frag_text_coord;

layout (binding = 1) uniform sampler2D text_sampler;

layout (location = 0) out vec4 out_color;

void main()
{
    // out_color = vec4(frag_color, 1.0);
    out_color = texture(text_sampler, frag_text_coord);
}