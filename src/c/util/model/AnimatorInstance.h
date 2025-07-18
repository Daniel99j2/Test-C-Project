//
// Created by dj on 29/06/2025.
//

#pragma once

#include <string>

#include "Animation.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <glm/fwd.hpp>

class AnimatorInstance {
public:
    struct ActiveAnim {
        const Animation *anim;
        float time = 0;
        bool playing = true;
        int priority = 0;
        std::unordered_set<std::string> allowedBones;
        bool loop = false;
    };

    void play(const Animation *anim, int priority = -100, bool loop = true);

    void pause(const std::string &animName);

    void resume(const std::string &animName);

    void cancel(const std::string &animName);

    void tick(float deltaTime);

    glm::mat4 getTransform(const std::string &boneName) const;

    glm::mat4 getFinalTransform(const std::string &boneName) const;

private:
    std::map<std::string, ActiveAnim> playingAnims;
    std::vector<Animation *> allowedAnimations;

    static glm::vec3 interpolate(const Keyframe &kf1, const Keyframe &kf2, float t);

    static const Keyframe *getPrevKey(const std::vector<Keyframe> &keyframes, float time);

    static const Keyframe *getNextKey(const std::vector<Keyframe> &keyframes, float time);
};
