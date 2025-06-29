//
// Created by dj on 29/06/2025.
//

#include "AnimatorInstance.h"
#include <cmath>
#include <stdexcept>

#include "libs/glm/ext/matrix_transform.hpp"

void AnimatorInstance::play(const Animation* anim, int priority, bool loop) {
    playingAnims[anim->name] = {
        anim, 0.0f, true, priority, std::move(anim->allowedBones)
    };
    if (loop) playingAnims[anim->name].loop = true;
}

void AnimatorInstance::pause(const std::string& animName) {
    if (playingAnims.contains(animName)) playingAnims[animName].playing = false;
}

void AnimatorInstance::resume(const std::string& animName) {
    if (playingAnims.contains(animName)) playingAnims[animName].playing = true;
}

void AnimatorInstance::cancel(const std::string& animName) {
    playingAnims.erase(animName);
}

void AnimatorInstance::tick(float deltaTime) {
    for (auto& [name, inst] : playingAnims) {
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
}

glm::mat4 AnimatorInstance::getTransform(const std::string& boneName) const {
    glm::vec3 position(0.0f), rotation(0.0f), scale(1.0f);
    int topPriority = -1000000;

    for (const auto& [name, inst] : playingAnims) {
        if (!inst.playing || !inst.allowedBones.contains(boneName)) continue;
        if (inst.priority < topPriority) continue;

        for (const auto& animator : inst.anim->animators) {
            if (animator.name != boneName) continue;

            for (Channel ch : {Channel::Position, Channel::Rotation, Channel::Scale}) {
                std::vector<Keyframe> filtered;
                for (const auto& kf : animator.keyframes)
                    if (kf.channel == ch)
                        filtered.push_back(kf);

                if (filtered.empty()) {
                    if (ch == Channel::Scale) scale = glm::vec3(1.0f);
                    continue;
                }

                const Keyframe* prev = getPrevKey(filtered, inst.time);
                const Keyframe* next = getNextKey(filtered, inst.time);

                // fallback so it doesnt goo all weird
                if (!prev || !next) {
                    if (ch == Channel::Scale) scale = glm::vec3(1.0f);
                    else if (ch == Channel::Rotation) rotation = glm::vec3(0.0f);
                    else if (ch == Channel::Position) position = glm::vec3(0.0f);
                    continue;
                }

                glm::vec3 val = prev->value;
                if (prev != next && next->time != prev->time) {
                    float t = (inst.time - prev->time) / (next->time - prev->time);
                    val = interpolate(*prev, *next, t);
                }

                if (ch == Channel::Position) position = val;
                else if (ch == Channel::Rotation) rotation = val;
                else if (ch == Channel::Scale) scale = val;
            }

            topPriority = inst.priority;
        }
    }

    auto transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, scale);
    return transform;
}

glm::mat4 AnimatorInstance::getFinalTransform(const std::string& boneName) const {
    if (!boneParents.contains(boneName)) {
        return getTransform(boneName); // root bone
    }
    return getFinalTransform(boneParents.at(boneName)) * getTransform(boneName);
}

glm::vec3 AnimatorInstance::interpolate(const Keyframe& kf1, const Keyframe& kf2, float t) {
    return glm::mix(kf1.value, kf2.value, t);
}

const Keyframe* AnimatorInstance::getPrevKey(const std::vector<Keyframe>& keyframes, float time) {
    const Keyframe* best = &keyframes.front();
    for (const auto& kf : keyframes) {
        if (kf.time <= time) best = &kf;
        else break;
    }
    return best;
}

const Keyframe* AnimatorInstance::getNextKey(const std::vector<Keyframe>& keyframes, float time) {
    for (const auto& kf : keyframes) {
        if (kf.time >= time) return &kf;
    }
    return &keyframes.back();
}
