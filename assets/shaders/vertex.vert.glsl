#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_text_coords;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 frag_text_coord;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 0.0, 1.0);
    frag_color = in_color;
    frag_text_coord = in_text_coords;
}