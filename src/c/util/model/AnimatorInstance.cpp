//
// Created by dj on 29/06/2025.
//

#include "AnimatorInstance.h"
#include <cmath>
#include <stdexcept>

#include "../Profiler.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/gtc/quaternion.hpp>

void AnimatorInstance::play(const Animation *anim, int priority, bool loop) {
    playingAnims[anim->name] = {
        anim, 0.0f, true, priority, anim->allowedBones
    };
    if (loop) playingAnims[anim->name].loop = true;
}

void AnimatorInstance::pause(const std::string &animName) {
    if (playingAnims.contains(animName)) playingAnims[animName].playing = false;
}

void AnimatorInstance::resume(const std::string &animName) {
    if (playingAnims.contains(animName)) playingAnims[animName].playing = true;
}

void AnimatorInstance::cancel(const std::string &animName) {
    playingAnims.erase(animName);
}

void AnimatorInstance::tick(float deltaTime) {
    Profiler::beginSection("Tick animation");
    for (auto &[name, inst]: playingAnims) {
        if (!inst.playing) continue;
        inst.time += deltaTime;

        if (inst.time > inst.anim->length) {
            if (inst.anim->loopMode == 1) {
                inst.time = std::fmod(inst.time, inst.anim->length);
            } else {
                if (inst.loop || inst.anim->loopMode == Loop) {
                    inst.time = 0;
                } else if (inst.anim->loopMode == Hold) {
                    inst.time = inst.anim->length;
                    inst.playing = false;
                } else if (inst.anim->loopMode == Once) {
                    inst.time = 0;
                    inst.playing = false;
                } else {
                    throw std::runtime_error("Invalid loop mode");
                }
            }
        }
    }
    Profiler::endSection("Tick animation");
}

glm::mat4 AnimatorInstance::getTransform(const std::string& boneName) const {
    Profiler::beginSection("Calculate animation");
    glm::vec3 position(0.0f), rotationEuler(0.0f), scale(1.0f);

    std::vector<std::pair<std::string, ActiveAnim>> playingAnims1(playingAnims.begin(), playingAnims.end());

    std::sort(playingAnims1.begin(), playingAnims1.end(), [](const std::pair<std::string, ActiveAnim> &a, const std::pair<std::string, ActiveAnim> &b) {
        return a.second.priority < b.second.priority;
    });

    for (const auto& [name, inst] : playingAnims1) {
        if (!inst.playing || !inst.allowedBones.contains(boneName)) continue;

        for (const auto& animator : inst.anim->keyframes) {
            if (animator.first != boneName) continue;

            for (Channel ch : {Channel::Position, Channel::Rotation, Channel::Scale}) {
                std::vector<Keyframe> filtered;
                for (const auto& kf : animator.second)
                    if (kf.channel == ch)
                        filtered.push_back(kf);

                if (filtered.empty()) {
                    continue;
                }

                const Keyframe* prev = getPrevKey(filtered, inst.time);
                const Keyframe* next = getNextKey(filtered, inst.time);

                if (!prev || !next) continue;

                glm::vec3 val = prev->value;
                if (prev != next && next->time != prev->time) {
                    float t = (inst.time - prev->time) / (next->time - prev->time);
                    val = interpolate(*prev, *next, t);
                }

                if (ch == Channel::Position) position = val;
                else if (ch == Channel::Rotation) rotationEuler = val;
                else if (ch == Channel::Scale) scale = val;
            }
        }
    }

    glm::mat4 transform(1.0f);
    transform = glm::translate(transform, position);
    glm::quat rotQuat = glm::quat(glm::radians(rotationEuler));
    transform *= glm::mat4(rotQuat);
    transform = glm::scale(transform, scale);
    Profiler::endSection("Calculate animation");
    return transform;
}

glm::vec3 AnimatorInstance::interpolate(const Keyframe& kf1, const Keyframe& kf2, float t) {
    if (kf1.interpolation == Interpolation::Smooth && kf2.interpolation == Interpolation::Smooth) {
        // Use quaternion slerp for smooth rotation interpolation
        glm::quat q1 = glm::quat(glm::radians(kf1.value));
        glm::quat q2 = glm::quat(glm::radians(kf2.value));
        glm::quat result = glm::slerp(q1, q2, t);
        return glm::degrees(glm::eulerAngles(result));
    } else {
        return glm::mix(kf1.value, kf2.value, t);
    }
}

const Keyframe *AnimatorInstance::getPrevKey(const std::vector<Keyframe> &keyframes, float time) {
    const Keyframe *best = &keyframes.front();
    for (const auto &kf: keyframes) {
        if (kf.time <= time) best = &kf;
        else break;
    }
    return best;
}

const Keyframe *AnimatorInstance::getNextKey(const std::vector<Keyframe> &keyframes, float time) {
    for (const auto &kf: keyframes) {
        if (kf.time >= time) return &kf;
    }
    return &keyframes.back();
}
