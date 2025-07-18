#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D base;
    sampler2D metal;
    sampler2D rough;
    sampler2D emissive;
    sampler2D normal;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;

uniform vec3 viewPos;
uniform Material material;
uniform PointLight pointLights[20];
uniform int pointLightsAmount;

uniform int debugRenderMode;

uniform DirectionalLight dirLight;

vec3 CalcPointLight(int index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, float metallic, float roughness);
vec3 CalcDirectionalLight(vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness);

void main() {
    vec4 baseColor1 = texture(material.base, TexCoords);
    if (baseColor1.a < 0.1) discard;
    vec3 baseColor = baseColor1.rgb;

    float metallic = texture(material.metal, TexCoords).r;
    float emissive = texture(material.emissive, TexCoords).r;
    float roughness = texture(material.rough, TexCoords).r;

    vec3 sampledNormal = texture(material.normal, TexCoords).rgb;
    sampledNormal = normalize(sampledNormal * 2.0 - 1.0);
    vec3 norm = normalize(TBN * sampledNormal);

    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

    result += CalcDirectionalLight(norm, viewDir, baseColor, metallic, roughness);

    for (int i = 0; i < pointLightsAmount; i++) {
        result += CalcPointLight(i, norm, FragPos, viewDir, baseColor, metallic, roughness);
    }

    result += baseColor * emissive;

    if (debugRenderMode == 0) FragColor = vec4(result, 1.0);
    else if (debugRenderMode == 1) FragColor = vec4(baseColor, 1.0);
    else if (debugRenderMode == 2) FragColor = vec4(norm, 1.0);
    else if (debugRenderMode == 3) FragColor = vec4(metallic, 0.0, 0.0, 1.0);
    else if (debugRenderMode == 4) FragColor = vec4(0.0, roughness, 0.0, 1.0);
    else if (debugRenderMode == 5) FragColor = vec4(0.0, 0.0, emissive, 1.0);
    else if (debugRenderMode == 6) FragColor = vec4(TexCoords.x, TexCoords.y, 0.0, 1.0);
}

vec3 CalcPointLight(int index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, float metallic, float roughness) {
    PointLight light = pointLights[index];
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(normal, halfwayDir), 0.0);
    float spec = pow(specAngle, mix(1.0, 128.0, 1.0 - roughness));

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float reflectance = mix(0.04, 1.0, metallic);
    float specStrength = metallic * (1.0 - roughness); // fully rough metals reflect less

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor * (1.0 - metallic); // metals don't diffuse
    vec3 specular = light.specular * spec * reflectance * specStrength;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 CalcDirectionalLight(vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness) {
    vec3 lightDir = normalize(-dirLight.direction); // light comes from opposite direction
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(normal, halfwayDir), 0.0);
    float spec = pow(specAngle, mix(1.0, 128.0, 1.0 - roughness));

    float reflectance = mix(0.04, 1.0, metallic);
    float specStrength = metallic * (1.0 - roughness);

    vec3 ambient = dirLight.ambient * baseColor;
    vec3 diffuse = dirLight.diffuse * diff * baseColor * (1.0 - metallic);
    vec3 specular = dirLight.specular * spec * reflectance * specStrength;

    return ambient + diffuse + specular;
}
