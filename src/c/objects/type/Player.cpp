//
// Created by dj on 22/06/2025.
//

#include "Player.h"

#include "src/c/util/ModelUtil.h"

Player::Player(const glm::vec3& vec)
    : LivingObject(vec) {
    maxHealth = 100;
    health = 100;
    damage = 10;
    speed = 1;
    type = "player";
    model = ModelUtil::getModel("player");
}

void Player::tick() {

}

void Player::move(bool w, bool a, bool s, bool d) {
    float max_speed = 200;
    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
    glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    glm::vec3 flatRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

    glm::vec3 inputDir(0.0f);
    if (w) inputDir += flatFront;
    if (a) inputDir -= flatRight;
    if (s) inputDir -= flatFront;
    if (d) inputDir += flatRight;

    if (glm::length(inputDir) > 0.0f)
        inputDir = glm::normalize(inputDir);

    velocity += inputDir * speed;


    if (glm::length(velocity) > max_speed)
        velocity = glm::normalize(velocity) * max_speed;
}