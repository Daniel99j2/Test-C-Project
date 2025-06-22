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

#include "util/ModelUtil.h"
#include "util/Shader.h"
//the bin folder contents needs to be copied!

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
int window_width = 800;
int window_height = 600;
int stride = 8;
boolean lockCursor = true;
boolean lockCursorP = false;
glm::vec3 playerPos(0.0f, 0.0f, -3.0f);
glm::vec2 playerRotation(0, 0);

int main() {
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Baseplate Test", NULL, NULL);
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

    Shader defaultShader("default");
    Shader lightEmitterShader("lighting");
    Shader skyboxShader("skybox");

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    vector<glm::vec3> pointLightPositions = {};
    vector<glm::vec3> pointLightColours = {};

    for (int i = 0; i < 20; ++i) {
        pointLightPositions.push_back(glm::vec3(GenericUtil::randomInt(-10, 10),  GenericUtil::randomInt(-10, 10),  GenericUtil::randomInt(-10, 10)));
        pointLightColours.push_back(glm::vec3(GenericUtil::randomFloat(0, 1, 2),  GenericUtil::randomFloat(0, 1, 2),  GenericUtil::randomFloat(0, 1, 2)));
    }

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int skyboxVAO;
    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLuint texture = RenderUtil::genTexture("src/resources/textures/test");
    GLuint texture1 = RenderUtil::genTexture("src/resources/textures/test2");
    GLuint texture2 = RenderUtil::genTexture("src/resources/textures/test3");

    GLuint skyboxTexture = RenderUtil::genTexture("src/resources/textures/skybox");

    defaultShader.use();

    RenderUtil::genOrLoadAtlas("src/resources/textures", "output/atlas_main.png", "output/atlas_main.json", "output/atlas_mer.png", "output/atlas_mer.json", false);
    glm::vec2 originalUV = {0.5f, 0.5f};
    glm::vec2 atlasUV = RenderUtil::getUV("test4.png", originalUV);

    Model test = ModelUtil::getModel("src/resources/models/hello.bbmodel");

    cout << atlasUV.x << " " << atlasUV.y << endl;

    float speed = 0.1f;
    glm::vec3 lightPos(1.5f, 1.0f, -2.3f);

    while (!glfwWindowShouldClose(window)) {
        glfwSetInputMode(window, GLFW_CURSOR, lockCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        glm::vec3 front;
        front.x = cos(glm::radians(playerRotation.y)) * sin(glm::radians(playerRotation.x));
        front.y = sin(glm::radians(playerRotation.y));
        front.z = cos(glm::radians(playerRotation.y)) * cos(glm::radians(playerRotation.x));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        glm::vec3 flatRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            playerPos += flatFront * speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            playerPos -= flatFront * speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            playerPos -= flatRight * speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            playerPos += flatRight * speed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            playerPos.y = playerPos.y + speed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            playerPos.y = playerPos.y - speed;
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            if (!lockCursorP) {
                lockCursorP = true;
                lockCursor = !lockCursor;
            }
        } else {
            lockCursorP = false;
        }

        //pointLightPositions[0] = glm::vec3(playerPos.x, playerPos.y - 1.0f, playerPos.z);

        auto model = glm::mat4(1.0f);
        auto view = glm::mat4(1.0f);
        view = glm::lookAt(playerPos + glm::vec3(0, 1.8, 0), playerPos + glm::vec3(0, 1.8, 0) + front, up);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        defaultShader.use();
        defaultShader.setVec3("viewPos", playerPos + glm::vec3(0, 1.8, 0));


        for(unsigned int i = 0; i < std::size(pointLightPositions); i++) {
            defaultShader.setVec3(("pointLights[" + std::to_string(i) + "].position"), pointLightPositions[0]);
            defaultShader.setVec3(("pointLights[" + std::to_string(i) + "].ambient"), 0.05f, 0.05f, 0.05f);
            defaultShader.setVec3(("pointLights[" + std::to_string(i) + "].diffuse"), pointLightColours[i]);
            defaultShader.setVec3(("pointLights[" + std::to_string(i) + "].specular"), 1.0f, 1.0f, 1.0f);
            defaultShader.setFloat(("pointLights[" + std::to_string(i) + "].constant"), 1.0f);
            defaultShader.setFloat(("pointLights[" + std::to_string(i) + "].linear"), 0.09f);
            defaultShader.setFloat(("pointLights[" + std::to_string(i) + "].quadratic"), 0.032f);
        }

        glm::vec3 sunDir = glm::vec3(0.0f, 0.0f, 0.0f);
        sunDir = GenericUtil::moveVec3(sunDir, -10,  1.0f + glfwGetTime() * 20.0f, 0);

        defaultShader.setVec3("dirLight.direction", sunDir);
        defaultShader.setVec3("dirLight.ambient", 0.01f, 0.01f, 0.01f);
        defaultShader.setVec3("dirLight.diffuse", 0.7f, 0.4f, 0.4f);
        defaultShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        defaultShader.setInt("pointLightsAmount", pointLightPositions.size());

        defaultShader.setMat4("projection", projection);
        defaultShader.setMat4("view", view);
        defaultShader.setMat4("model", model);

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        test.Draw(defaultShader);
        // glDrawArrays(GL_TRIANGLES, 0, 36);

        lightEmitterShader.use();
        lightEmitterShader.setMat4("projection", projection);
        lightEmitterShader.setMat4("view", view);

        glBindVertexArray(lightCubeVAO);


        for(unsigned int i = 0; i < std::size(pointLightPositions); i++) {
            glm::mat4 model1 = glm::mat4(1.0f);
            model1 = glm::translate(model1, pointLightPositions[i]);
            model1 = glm::scale(model1, glm::vec3(0.2f));
            lightEmitterShader.setMat4("model", model1);
            lightEmitterShader.setVec3("lightColour", pointLightColours[i]);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

//temp
        skyboxShader.use();
        skyboxShader.setInt("asset", 0);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setMat4("view", view);
        float test1 = 1.0f/fmod(glfwGetTime(), 10.0f);
        skyboxShader.setFloat("lightness", test1);
        auto model2 = glm::mat4(1.0f);
        model2 = glm::scale(model2, glm::vec3(-100));
        skyboxShader.setMat4("model", model2);

        glBindVertexArray(skyboxVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skyboxTexture);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthMask(TRUE);

        glfwSwapBuffers(window);
        glfwPollEvents();

        string title = "Baseplate - Pos: ";
        title = title.append(to_string(playerPos.x));
        title = title.append(" ");
        title = title.append(to_string(playerPos.y));
        title = title.append(" ");
        title = title.append(to_string(playerPos.z));
        glfwSetWindowTitle(window, title.c_str());

        Sleep(1000/60);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    window_width = width;
    window_height = height;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (lockCursor) {
        int windowX;
        int windowY;
        float maxSensitivity = 50.0f;
        float sensitivity = 50.0f;
        glfwGetWindowPos(window, &windowX, &windowY);

        //we dont wan NaN from /0
        playerRotation.x -= (xpos-(windowX+window_width / 2)) / max(maxSensitivity - sensitivity, 1.0f) / 20;
        playerRotation.y -= (ypos-(windowY+window_height / 2)) / max(maxSensitivity - sensitivity, 1.0f) / 20;

        playerRotation.x = glm::mod(playerRotation.x, 360.0f);
        playerRotation.y = glm::clamp(playerRotation.y, -89.0f, 89.0f);

        glfwSetCursorPos(window, windowX+window_width / 2, windowY+window_height / 2);
    }
}