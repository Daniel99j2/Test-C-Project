#include <iostream>
#include <chrono>
#include <random>
#include "../../libs/glew/include/GL/glew.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include "util/RenderUtil.h"
#include <windows.h>
#include "../../../libs/glm/vec3.hpp"
#include "../../../libs/glm/mat4x4.hpp"
#include "../../../libs/glm/ext/matrix_transform.hpp"
#include "../../../libs/glm/ext/matrix_clip_space.hpp"
#include "../../../libs/glm/glm.hpp"
#include "../../../libs/glm/gtc/type_ptr.hpp"
#include "util/GenericUtil.h"
#include <string>
#include "../../../libs/json.hpp"
#include "objects/PhysicsEngine.h"
#include "util/GameConstants.h"
#include "util/Model.h"

#include "util/ModelUtil.h"
#include "util/Shader.h"
#include "objects/GameObject.h"
#include "objects/type/Player.h"
#include "objects/type/TestObject.h"
#include "world/World.h"
//the bin folder contents needs to be copied!

using namespace std;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

int window_width = 800;
int window_height = 600;
int stride = 8;
boolean lockCursor = true;
boolean lockCursorP = false;
std::map<std::string, std::string> args;

int main(int argc, char *argv[]) {
    cout << "Game loading..." << endl;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.starts_with("--")) {
            std::string key = arg.substr(2);
            if (i + 1 < argc && std::string(argv[i + 1]).starts_with("--") == false) {
                args[key] = argv[++i];
            } else {
                args[key] = "true";
            }
        }
    }

    if (!glfwInit()) {
        return -1;
    }

    float startTime = glfwGetTime();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "Baseplate Test", NULL, NULL);
    if (!window) {
        std::cerr << "Couldn't create the window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glEnable(GL_DEPTH_TEST);

    if (const GLenum err = glewInit(); err != GLEW_OK) {
        std::cerr << "GLEW init failed: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    vector<glm::vec3> pointLightPositions = {};
    vector<glm::vec3> pointLightColours = {};

    for (int i = 0; i < 20; ++i) {
        pointLightPositions.push_back(glm::vec3(GenericUtil::randomInt(-10, 10), GenericUtil::randomInt(-10, 10),
                                                GenericUtil::randomInt(-10, 10)));
        pointLightColours.push_back(glm::vec3(GenericUtil::randomFloat(0, 1, 2), GenericUtil::randomFloat(0, 1, 2),
                                              GenericUtil::randomFloat(0, 1, 2)));
    }

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    unsigned int skyboxVAO;
    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    if (GameConstants::wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLuint texture = RenderUtil::genTexture("src/resources/textures/test");
    GLuint texture1 = RenderUtil::genTexture("src/resources/textures/test2");
    GLuint texture2 = RenderUtil::genTexture("src/resources/textures/test3");

    GameConstants::defaultShader = Shader("default");
    GameConstants::skyboxShader = Shader("skybox");
    GameConstants::lightEmitterShader = Shader("lighting");

    GLuint skyboxTexture = RenderUtil::genTexture("src/resources/textures/skybox");

    GameConstants::defaultShader.use();

    RenderUtil::genOrLoadAtlas("src/resources/textures", "output/atlases/atlas_main.png",
                               "output/atlases/atlas_main.json", "output/atlases/atlas_mer.png",
                               "output/atlases/atlas_mer.json", args.contains("regenAtlas") || args.contains("regenAll"));

    ModelUtil::loadModels(args.contains("regenAtlas") || args.contains("regenModels") || args.contains("regenAll"));

    float speed = 0.1f;
    glm::vec3 lightPos(1.5f, 1.0f, -2.3f);

    cout << "Game loaded! \nGame took " << glfwGetTime() - startTime << " seconds to start!" << endl;

    auto world = World();
    GameConstants::player = std::make_shared<Player>(glm::vec3(1, 1, 1));;
    world.addObject(std::static_pointer_cast<GameObject>(GameConstants::player));

    auto g = std::make_shared<TestObject>(glm::vec3(1, 3, 1));
    world.addObject(std::static_pointer_cast<GameObject>(g));

    while (!glfwWindowShouldClose(window)) {
        glfwSetInputMode(window, GLFW_CURSOR, lockCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        glm::vec3 front;
        front.x = cos(glm::radians(GameConstants::player->pitch)) * sin(glm::radians(GameConstants::player->yaw));
        front.y = sin(glm::radians(GameConstants::player->pitch));
        front.z = cos(glm::radians(GameConstants::player->pitch)) * cos(glm::radians(GameConstants::player->yaw));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        glm::vec3 flatRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            GameConstants::player->position += flatFront * speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            GameConstants::player->position -= flatFront * speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            GameConstants::player->position -= flatRight * speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            GameConstants::player->position += flatRight * speed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            GameConstants::player->position.y = GameConstants::player->position.y + speed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            GameConstants::player->position.y = GameConstants::player->position.y - speed;
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            if (!lockCursorP) {
                lockCursorP = true;
                lockCursor = !lockCursor;
            }
        } else {
            lockCursorP = false;
        }

        world.tick();

        auto model = glm::mat4(1.0f);
        auto view = glm::mat4(1.0f);
        view = glm::lookAt(GameConstants::player->position + glm::vec3(0, 1.8, 0), GameConstants::player->position + glm::vec3(0, 1.8, 0) + front, up);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        GameConstants::defaultShader.use();
        GameConstants::defaultShader.setVec3("viewPos", GameConstants::player->position + glm::vec3(0, 1.8, 0));

        for (unsigned int i = 0; i < std::size(pointLightPositions); i++) {
            GameConstants::defaultShader.setVec3(("pointLights[" + std::to_string(i) + "].position"),
                                                 pointLightPositions[0]);
            GameConstants::defaultShader.setVec3(("pointLights[" + std::to_string(i) + "].ambient"), 0.05f, 0.05f,
                                                 0.05f);
            GameConstants::defaultShader.setVec3(("pointLights[" + std::to_string(i) + "].diffuse"),
                                                 pointLightColours[i]);
            GameConstants::defaultShader.setVec3(("pointLights[" + std::to_string(i) + "].specular"), 1.0f, 1.0f, 1.0f);
            GameConstants::defaultShader.setFloat(("pointLights[" + std::to_string(i) + "].constant"), 1.0f);
            GameConstants::defaultShader.setFloat(("pointLights[" + std::to_string(i) + "].linear"), 0.09f);
            GameConstants::defaultShader.setFloat(("pointLights[" + std::to_string(i) + "].quadratic"), 0.032f);
        }

        glm::vec3 sunDir = glm::vec3(0.0f, 0.0f, 0.0f);
        sunDir = GenericUtil::moveVec3(sunDir, -10, 1.0f + glfwGetTime() * 20.0f, 0);

        GameConstants::defaultShader.setVec3("dirLight.direction", sunDir);
        GameConstants::defaultShader.setVec3("dirLight.ambient", 0.01f, 0.01f, 0.01f);
        GameConstants::defaultShader.setVec3("dirLight.diffuse", 0.7f, 0.4f, 0.4f);
        GameConstants::defaultShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        GameConstants::defaultShader.setInt("pointLightsAmount", pointLightPositions.size());

        GameConstants::defaultShader.setMat4("projection", projection);
        GameConstants::defaultShader.setMat4("view", view);
        GameConstants::defaultShader.setMat4("model", model);

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        world.drawWorld();

        GameConstants::lightEmitterShader.use();
        GameConstants::lightEmitterShader.setMat4("projection", projection);
        GameConstants::lightEmitterShader.setMat4("view", view);

        glBindVertexArray(lightCubeVAO);


        for (unsigned int i = 0; i < std::size(pointLightPositions); i++) {
            glm::mat4 model1 = glm::mat4(1.0f);
            model1 = glm::translate(model1, pointLightPositions[i]);
            model1 = glm::scale(model1, glm::vec3(0.2f));
            GameConstants::lightEmitterShader.setMat4("model", model1);
            GameConstants::lightEmitterShader.setVec3("lightColour", pointLightColours[i]);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //temp
        GameConstants::skyboxShader.use();
        GameConstants::skyboxShader.setInt("asset", 0);
        GameConstants::skyboxShader.setMat4("projection", projection);
        GameConstants::skyboxShader.setMat4("view", view);
        float test1 = 1.0f / fmod(glfwGetTime(), 10.0f);
        GameConstants::skyboxShader.setFloat("lightness", test1);
        auto model2 = glm::mat4(1.0f);
        model2 = glm::scale(model2, glm::vec3(-100));
        GameConstants::skyboxShader.setMat4("model", model2);

        glBindVertexArray(skyboxVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skyboxTexture);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthMask(TRUE);

        glfwSwapBuffers(window);
        glfwPollEvents();

        string title = ("Baseplate test game - Pos: " + to_string(GameConstants::player->position.x) + " " + to_string(GameConstants::player->position.y) + " " + to_string(GameConstants::player->position.z) + " Pitch: " + to_string(GameConstants::player->pitch) + " Yaw: " + to_string(GameConstants::player->yaw));
        glfwSetWindowTitle(window, title.c_str());

        GameConstants::physicsEngine.simulate(0.1f);

        Sleep(1000 / GameConstants::targetFPS);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    window_width = width;
    window_height = height;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (lockCursor) {
        int windowX;
        int windowY;
        float maxSensitivity = 50.0f;
        float sensitivity = 50.0f;
        glfwGetWindowPos(window, &windowX, &windowY);

        //we dont wan NaN from /0
        GameConstants::player->yaw -= (xpos - (windowX + window_width / 2)) / max(maxSensitivity - sensitivity, 1.0f) / 20;
        GameConstants::player->pitch -= (ypos - (windowY + window_height / 2)) / max(maxSensitivity - sensitivity, 1.0f) / 20;

        GameConstants::player->yaw = glm::mod(GameConstants::player->yaw, 360.0f);
        GameConstants::player->pitch = glm::clamp(GameConstants::player->pitch, -89.0f, 89.0f);

        glfwSetCursorPos(window, windowX + window_width / 2, windowY + window_height / 2);
    }
}