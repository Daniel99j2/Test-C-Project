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

#include "util/Shader.h"

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
int window_width = 800;
int window_height = 600;
glm::vec3 playerPos(0.0f, 0.0f, -3.0f);
glm::vec2 playerRotation(0, 0);

int main() {
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Test", NULL, NULL);
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

    Shader shader("src/resources/shader/vertex_shader.glsl", "src/resources/shader/fragment_shader.glsl");

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    //5 is the data length
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLuint texture = RenderUtil::genTexture(100, 100, 3, "src/resources/textures/test.png");
    GLuint texture1 = RenderUtil::genTexture(100, 100, 3, "src/resources/textures/test2.png");

    shader.use();
    shader.setInt("texture0", 0);
    shader.setInt("texture2", 1);

    float speed = 0.1f;

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


        int modelLoc = glGetUniformLocation(shader.ID, "model");
        int viewLoc = glGetUniformLocation(shader.ID, "view");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        shader.setMat4("projection", projection);


        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture1);

        shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();

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