#include <iostream>
#include <chrono>
#include <random>
#include "../../libs/glew/include/GL/glew.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include "util/RenderUtil.h"
#include "util/GenericUtil.h"
#include <windows.h>

using namespace std;

int main() {
    for (int i = 0; i < 10; ++i) {
        cout << GenericUtil::randomInt(0, 3) << endl;
    }

    string name;
    string job;
    int power = 0;

    // cout << "Enter your name:" << endl;
    // cin >> name;
    // cout << "Hey, "+name+". Nice name!" << endl;
    // cout << "What do you do?" << endl;
    // cin >> job;
    // cout << "Hey, "+name+". Your job is "+job+"!" << endl;

    power = GenericUtil::randomInt(1, 1000);

    cout << "lets start" << endl;

    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (const GLenum err = glewInit(); err != GLEW_OK) {
        std::cerr << "GLEW init failed: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    GLuint shaderProgram = RenderUtil::createShaderProgram("src/resources/shader/vertex_shader.glsl", "src/resources/shader/fragment_shader.glsl");

    // Create a vertex buffer object (VBO)
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create a vertex array object (VAO)
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    ShowCursor(FALSE);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Render triangle
        glUseProgram(shaderProgram);
        glUniform4f(glGetUniformLocation(shaderProgram, "inColour"), 1, sin(glfwGetTime()) / 2.0f + 0.5f, 0, 1);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
            for (int keyCode = 0; keyCode < 256; ++keyCode) {
                if (GetAsyncKeyState(keyCode) & 0x8000) {
                    char keyChar = static_cast<char>(keyCode);
                    cout << "Pressed Key: " << keyChar
                            << " (ASCII value: " << keyCode << ")"
                            << endl;
                }
            }

            POINT p;
            if (GetCursorPos(&p)) {
                cout << p.x << endl;
            }
        }

        Sleep(100);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}