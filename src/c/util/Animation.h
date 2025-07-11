//
// Created by dj on 28/06/2025.
//
#pragma once
#include <unordered_set>
#include <vector>
#include <glm/glm.hpp>

enum Channel { Position, Rotation, Scale };
enum Interpolation { Linear, Smooth };
enum LoopMode { Once, Loop, Hold };

struct Vec3 {
    float x, y, z;
};

struct Keyframe {
    float time;
    Channel channel;
    Interpolation interpolation;
    glm::vec3 value;
};

struct Animator {
    std::string name;
    std::vector<Keyframe> keyframes;
};

struct Animation {
    std::string name;
    float length;
    LoopMode loopMode;
    std::vector<Animator> animators;
    std::unordered_set<std::string> allowedBones;
};
