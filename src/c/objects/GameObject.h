#pragma once
#include <string>
#include <vector>
#include <memory>

#include "../util/AnimatorInstance.h"
#include "../util/Model.h"
#include "../util/Shader.h"

enum class ShapeType {
    Rectangle,
    Sphere,
    Cylinder
};

class CollisionPart {
public:
    ShapeType shape;
    glm::vec3 position;
    glm::vec3 size;
};

struct Collision {
    std::shared_ptr<class GameObject> other;
    glm::vec3 point;
};

class GameObject {
public:
    glm::vec3 position;
    glm::vec3 velocity = glm::vec3(0);
    float mass;
    float gravity;
    bool isStatic = false;
    bool pushable = false;
    std::vector<Collision> collisions;
    std::vector<CollisionPart>* collisionParts;

    void applySlowdown(float drag);
    void update(float dt);

    Model* model;
    Shader shader;
    AnimatorInstance animator = AnimatorInstance();
    std::string type;
    float pitch = 0;
    float yaw = 0;
    glm::mat4 transform = glm::mat4(1.0f);
    int id = -1;

    void draw(float deltaTime);
    void baseTick();
    virtual void tick() = 0;
    virtual ~GameObject() = default;

    explicit GameObject(glm::vec3 vec);
    GameObject(const GameObject&) = default;
    GameObject& operator=(const GameObject&) = default;

private:
    GameObject();
};