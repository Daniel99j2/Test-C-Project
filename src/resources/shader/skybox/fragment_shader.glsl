#version 330 core

in vec3 vDirection;
out vec4 fragColor;

uniform vec3 playerPosition;

uniform float time;
uniform float cirrus;   // 0.0 to 1.0
uniform float cumulus;  // 0.0 to 1.0

const float Br = 0.0005;
const float Bm = 0.0003;
const float g = 0.9200;
const vec3 nitrogen = vec3(0.650, 0.570, 0.475);
const vec3 Kr = Br / pow(nitrogen, vec3(4.0));
const vec3 Km = Bm / pow(nitrogen, vec3(0.84));

float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f); // smootherstep interpolation

    float n = dot(p, vec3(1.0, 157.0, 113.0));

    return mix(mix(mix(hash(n +   0.0), hash(n +   1.0), f.x),
                   mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
               mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
                   mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
}

const mat3 m = mat3(0.0, 1.60, 1.20,
-1.6, 0.72, -0.96,
-1.2, -0.96, 1.28);

float fbm(vec3 p) {
    float f = 0.0;
    f += noise(p) / 2.0; p = m * p * 1.1;
    f += noise(p) / 4.0; p = m * p * 1.2;
    f += noise(p) / 6.0; p = m * p * 1.3;
    f += noise(p) / 12.0; p = m * p * 1.4;
    f += noise(p) / 24.0;
    return f;
}

void main() {
    vec3 dir = normalize(vDirection);
    vec3 fsun = vec3(0.0, sin(time * 0.01), cos(time * 0.01));

    vec3 offset = playerPosition * 0.1;
    vec3 swirl = vec3(sin(dir.y * 4.0 + time), cos(dir.x * 4.0 + time * 0.3), sin(dir.z + time * 0.2)) * 0.5;

    float mu = dot(dir, normalize(fsun));
    float rayleigh = 3.0 / (8.0 * 3.14159) * (1.0 + mu * mu);
    vec3 mie = (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm);

    vec3 day_extinction = exp(-exp(-((dir.y + fsun.y * 4.0) * (exp(-dir.y * 16.0) + 0.1) / 80.0) / Br)
                              * (exp(-dir.y * 16.0) + 0.1) * Kr / Br)
    * exp(-dir.y * exp(-dir.y * 8.0) * 4.0)
    * exp(-dir.y * 2.0) * 4.0;

    vec3 night_extinction = vec3(1.0 - exp(fsun.y)) * 0.2;
    vec3 extinction = mix(day_extinction, night_extinction, -fsun.y * 0.2 + 0.5);

    vec3 result = rayleigh * mie * extinction;

    // Cirrus
    float density = smoothstep(1.0 - cirrus, 1.0, fbm(dir / dir.y * 2.0 + time * 0.05 + offset + swirl)) * 0.3;
    result = mix(result, extinction * 4.0, density * max(dir.y, 0.0));

    // Cumulus
    for (int i = 0; i < 10; i++) {
        vec3 p = (0.7 + float(i) * 0.01) * dir / dir.y + time * 0.3 + offset + swirl;
        float d = smoothstep(1.0 - cumulus, 1.0, fbm(p));
        result = mix(result, extinction * d * 5.0, min(d, 1.0) * max(dir.y, 0.0));
    }

    // Dithering
    result += noise(dir * 1000.0) * 0.01;

    // Fade to black at bottom
    float fade = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0); // -1 → 0, 1 → 1
    result *= fade;

    fragColor = vec4(result, 1.0);
}
