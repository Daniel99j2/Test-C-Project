#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D hdrBuffer;

void main() {
    const float gamma = 2.2;
    const float exposure = 1.0;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
    //FragColor = vec4(hdrColor, 1.0);
}