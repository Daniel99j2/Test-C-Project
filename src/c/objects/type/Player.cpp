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
    speed = 5;
    type = "player";
    model = ModelUtil::getModel("fplayer");
}

void Player::tick() {

}