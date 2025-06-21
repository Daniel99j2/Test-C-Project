#version 330 core

in vec2 TexCoords;

uniform sampler2D asset;

out vec4 FragColor;

void main() {
    FragColor = vec4(texture(asset, TexCoords));
}