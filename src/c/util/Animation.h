//
// Created by dj on 28/06/2025.
//
#pragma once

enum class Channel : uint8_t { Position, Rotation, Scale };
enum class Interpolation : uint8_t { Linear };

struct Vec3 {
    float x, y, z;
};

struct Keyframe {
    float time;
    Channel channel;
    Interpolation interpolation;
    Vec3 value;
};

struct Animator {
    std::string name;
    std::vector<Keyframe> keyframes;
};

struct Animation {
    std::string name;
    float length;
    uint8_t loopMode; // 0 = none, 1 = loop, etc.
    std::vector<Animator> animators;
};
