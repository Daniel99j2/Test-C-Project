#version 330 core

layout (location = 0) in vec3 position;

out vec3 vDirection;
out vec3 playerPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(position, 1.0);
    vDirection = worldPos.xyz;

    mat4 viewNoTranslation = mat4(mat3(view));
    playerPosition = -transpose(mat3(view)) * vec3(view[3]);
    gl_Position = projection * viewNoTranslation * worldPos;
}
