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

#include "util/Shader.h"
//the bin folder contents needs to be copied!

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
int window_width = 800;
int window_height = 600;
int stride = 8;
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

    Shader defaultShader("src/resources/shader/default/vertex_shader.glsl", "src/resources/shader/default/fragment_shader.glsl");
    Shader lightEmitterShader("src/resources/shader/lighting/vertex_shader.glsl", "src/resources/shader/lighting/fragment_shader.glsl");

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

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLuint texture = RenderUtil::genTexture("src/resources/textures/test.png");
    GLuint texture1 = RenderUtil::genTexture("src/resources/textures/test2.png");
    GLuint texture2 = RenderUtil::genTexture("src/resources/textures/test3.png");

    defaultShader.use();
    defaultShader.setInt("material.diffuse", 0);

    float speed = 0.1f;
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

    while (!glfwWindowShouldClose(window)) {
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


        auto model = glm::mat4(1.0f);
        auto view = glm::mat4(1.0f);
        view = glm::lookAt(playerPos, playerPos + front, up);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        defaultShader.use();
        defaultShader.setVec3("viewPos", playerPos);
        defaultShader.setInt("material.diffuse", 0);
        defaultShader.setInt("material.specular", 1);
        defaultShader.setInt("material.emissive", 2);
        defaultShader.setFloat("material.shininess", 64.0f);

        defaultShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        defaultShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        defaultShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        defaultShader.setVec3("light.position", lightPos);


        defaultShader.setMat4("projection", projection);
        defaultShader.setMat4("view", view);
        defaultShader.setMat4("model", model);

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        lightEmitterShader.use();
        lightEmitterShader.setMat4("projection", projection);
        lightEmitterShader.setMat4("view", view);
        auto model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, lightPos);
        model1 = glm::scale(model1, glm::vec3(0.2f)); // a smaller cube
        // lightEmitterShader.setMat4("model", model1);

        // for(unsigned int i = 0; i < 10; i++)
        // {
        //     glm::mat4 model = glm::mat4(1.0f);
        //     //model = glm::translate(model, cubePositions[i]);
        //     float angle = 20.0f * i;
        //     model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        //     lightEmitterShader.setMat4("model", model);
        //
        //     glDrawArrays(GL_TRIANGLES, 0, 36);
        // }

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

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
    int windowX;
    int windowY;
    glfwGetWindowPos(window, &windowX, &windowY);

    playerRotation.x -= (xpos-(windowX+window_width / 2)) / 50.0f;
    playerRotation.y -= (ypos-(windowY+window_height / 2)) / 50.0f;

    playerRotation.x = glm::mod(playerRotation.x, 360.0f);
    playerRotation.y = glm::clamp(playerRotation.y, -89.0f, 89.0f);

    glfwSetCursorPos(window, windowX+window_width / 2, windowY+window_height / 2);
}