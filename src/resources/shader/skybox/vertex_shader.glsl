#version 330 core

layout (location = 0) in vec3 position;

out vec3 vDirection;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(position, 1.0);
    vDirection = worldPos.xyz;

    gl_Position = projection * view * worldPos;
}
