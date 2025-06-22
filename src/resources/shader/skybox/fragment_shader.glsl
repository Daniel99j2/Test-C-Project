#version 330 core

in vec2 TexCoords;

uniform sampler2D asset;
uniform float lightness;

out vec4 FragColor;

void main() {
    FragColor = vec4(texture(asset, TexCoords)) * lightness;
}