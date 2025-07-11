#include "PreImports.h"
#include "util/RenderUtil.h"
#include "util/GenericUtil.h"
#include "util/Logger.h"
#include "util/GameConstants.h"
#include "util/Model.h"
#include "util/ModelUtil.h"
#include "util/Shader.h"
#include "objects/GameObject.h"
#include "objects/type/Player.h"
#include "objects/type/SimpleObject.h"
#include "util/Keybind.h"
#include "util/Keybinds.h"
#include "util/Profiler.h"
#include "world/World.h"

#include <stb_image.h>

//the bin folder contents needs to be copied!

using namespace std;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

int stride = 8;
std::map<std::string, std::string> args;
std::chrono::high_resolution_clock::time_point lastFrameTime;

struct Light {
    glm::vec3 position;
    float constant;
    float linear;
    float quadratic;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

std::vector<Light> lights;

std::map<std::string, bool> debugCheckboxes;

int main(int argc, char *argv[]) {
    Logger logger;
    cout << "[INFO] [Main] Game loading..." << endl;

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

    GameConstants::debug = args.contains("debug");

    if (!glfwInit()) {
        return -1;
    }

    float startTime = glfwGetTime();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow *window = glfwCreateWindow(GameConstants::window_width, GameConstants::window_height, "Baseplate Test", NULL, NULL);
    GameConstants::window = window;
    if (!window) {
        std::cerr << "[ERROR] [Main] Couldn't create the window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glEnable(GL_DEPTH_TEST);

    if (const GLenum err = glewInit(); err != GLEW_OK) {
        std::cerr << "[ERROR] [Main] GLEW init failed: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui_ImplGlfw_InitForOpenGL(window, true);

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

    for (int i = 0; i < 4; ++i) {
        glm::vec3 colour = glm::vec3(GenericUtil::randomFloat(0, 1, 2), GenericUtil::randomFloat(0, 1, 2),
                      GenericUtil::randomFloat(0, 1, 2));
        lights.push_back(Light(
            glm::vec3(GenericUtil::randomInt(-5, 5), GenericUtil::randomInt(-5, 5),
                      GenericUtil::randomInt(-5, 5)),
            1, 0.09f, 0.032f,
            colour/100.0f,
            colour,
            colour/2.0f
        ));
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

    GLFWimage *icon = RenderUtil::getImageData("src/resources/textures/noatlas/icon");
    glfwSetWindowIcon(window, 1, icon);
    stbi_image_free(icon->pixels);
    delete icon;

    GameConstants::defaultShader = Shader("default");
    GameConstants::skyboxShader = Shader("skybox");
    GameConstants::lightEmitterShader = Shader("lighting");

    GameConstants::defaultShader.use();

    RenderUtil::genOrLoadAtlas("src/resources/textures", "output/atlases/atlas_main.png",
                               "output/atlases/atlas_main.json", "output/atlases/atlas_mer.png",
                               "output/atlases/atlas_mer.json",
                               args.contains("regenAtlas") || args.contains("regenAll"));

    ModelUtil::loadModels();

    //Model* skybox = ModelUtil::getModel("skybox");

    glm::vec3 lightPos(1.5f, 1.0f, -2.3f);

    cout << "[INFO] [Game] Game loaded!" << endl;
    cout << "[INFO] [Game] Game took " << glfwGetTime() - startTime << " seconds to start!" << endl;

    GameConstants::world = World();
    GameConstants::player = std::make_shared<Player>(glm::vec3(5, 10, 0));;
    GameConstants::world.addObject(std::static_pointer_cast<GameObject>(GameConstants::player));
    GameConstants::player->gravity = 0;

    auto g = std::make_shared<SimpleObject>(glm::vec3(0, 0, 0));
    GameConstants::world.addObject(std::static_pointer_cast<GameObject>(g));
    g->gravity = 0;

    auto g1 = std::make_shared<SimpleObject>(glm::vec3(10, 2, 0));
    GameConstants::world.addObject(std::static_pointer_cast<GameObject>(g1));
    g1->gravity = 0;
    g1->model = ModelUtil::getModel("blender");

    //g->animator.play(&g->model.animations[0]);
    //g->animator.play(&g->model.animations[1]);

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    //anti aliasing - stops the jagged edges
    //glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    //gamma correction
    glEnable(GL_FRAMEBUFFER_SRGB);

    glfwSwapInterval(1);

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* vendor   = glGetString(GL_VENDOR);
    printf("Renderer: %s\nVendor: %s\n", renderer, vendor);

    while (!glfwWindowShouldClose(window)) {
        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
        lastFrameTime = now;

        auto oldProfilerResults = Profiler::getResults();

        Profiler::fpsHistory[Profiler::fpsIndex % 100] = 1.0f / std::max(deltaTime, 0.0001f);
        Profiler::fpsIndex++;

        Profiler::beginFrame();

        Profiler::beginSection("ImGui");
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Begin("Inventory");
        ImGui::Text("You have %d apples", 5);
        ImGui::End();

        ImGui::Begin("Debug options");
        ImGui::SeparatorText("Game");
        ImGui::Checkbox("Wireframe", &GameConstants::wireframe);
        ImGui::InputInt("FPS", &GameConstants::targetFPS);

        ImGui::SeparatorText("Player");
        if (ImGui::Button("Launch"))
            for (const auto &object: GameConstants::world.getObjects()) {
                object->velocity = glm::vec3(5, 5, 5);
            };
        ImGui::End();

        ImGui::Begin("Profiler");
        ImGui::PlotLines("FPS", Profiler::fpsHistory, 100, 0, nullptr, 0.0f, 120, ImVec2(0, 80));

        for (const auto& [name, res] : oldProfilerResults) {
            ImGui::Text("%s: %.3f s (%.3f ms avg, %d calls)", name.c_str(),
                        res.totalTime / 1000.0f,
                        res.totalTime / res.callCount,
                        res.callCount);
        }
        ImGui::End();
        Profiler::endSection("ImGui");

        logger.render();

        if (GameConstants::wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        Profiler::beginSection("Input");
        for (auto &k: GameConstants::keybindsManager.keybinds) k->update(window);
        glfwSetInputMode(window, GLFW_CURSOR, !GameConstants::keybindsManager.TOGGLE_CURSOR->isPressd() && !GameConstants::debugging
                                                  ? GLFW_CURSOR_DISABLED
                                                  : GLFW_CURSOR_NORMAL);

        glm::vec3 front;
        front.x = cos(glm::radians(GameConstants::player->pitch)) * sin(glm::radians(GameConstants::player->yaw));
        front.y = sin(glm::radians(GameConstants::player->pitch));
        front.z = cos(glm::radians(GameConstants::player->pitch)) * cos(glm::radians(GameConstants::player->yaw));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        glm::vec3 flatRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
        Profiler::endSection("Input");

        Profiler::beginSection("World Tick");
        GameConstants::world.tick(deltaTime);
        Profiler::endSection("World Tick");

        auto model = glm::mat4(1.0f);
        auto view = glm::mat4(1.0f);
        if (!GameConstants::keybindsManager.TOGGLE_CAMERA->isPressd()) {
            view = glm::lookAt(glm::vec3(8, 8, 8), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        } else {
            view = glm::lookAt(GameConstants::player->position + glm::vec3(0, 1.8, 0),
                               GameConstants::player->position + glm::vec3(0, 1.8, 0) + front, up);
        }

        glm::mat4 projection;
        //dont divide by 0
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(GameConstants::window, &fbWidth, &fbHeight);
        float aspect = (fbHeight > 0) ? (float)fbWidth / fbHeight : 4.0f / 3.0f;

        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        glEnable(GL_DEPTH_TEST);

        Profiler::beginSection("Main");

        Profiler::beginSection("Lighting");
        GameConstants::defaultShader.use();
        GameConstants::defaultShader.setVec3("viewPos", GameConstants::keybindsManager.TOGGLE_CAMERA->isPressd()
                                                            ? glm::vec3(8, 8, 8)
                                                            : GameConstants::player->position + glm::vec3(0, 1.8, 0));

        for(int i = 0; i < lights.size(); ++i) {
            std::string idx = std::to_string(i);
            GameConstants::defaultShader.setVec3("pointLights[" + idx + "].position", lights[i].position);
            GameConstants::defaultShader.setVec3("pointLights[" + idx + "].ambient", lights[i].ambient);
            GameConstants::defaultShader.setVec3("pointLights[" + idx + "].diffuse", lights[i].diffuse);
            GameConstants::defaultShader.setVec3("pointLights[" + idx + "].specular", lights[i].specular);
            GameConstants::defaultShader.setFloat("pointLights[" + idx + "].constant", lights[i].constant);
            GameConstants::defaultShader.setFloat("pointLights[" + idx + "].linear", lights[i].linear);
            GameConstants::defaultShader.setFloat("pointLights[" + idx + "].quadratic", lights[i].quadratic);
        }

        GameConstants::defaultShader.setInt("pointLightsAmount", lights.size());

        Profiler::endSection("Lighting");

        GameConstants::defaultShader.setMat4("projection", projection);
        GameConstants::defaultShader.setMat4("view", view);
        GameConstants::defaultShader.setMat4("model", model);

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Profiler::beginSection("Render");
        GameConstants::world.drawWorld(deltaTime);
        Profiler::endSection("Render");

        GameConstants::lightEmitterShader.use();
        GameConstants::lightEmitterShader.setMat4("projection", projection);
        GameConstants::lightEmitterShader.setMat4("view", view);

        glBindVertexArray(lightCubeVAO);


        for (unsigned int i = 0; i < std::size(lights); i++) {
            glm::mat4 model1 = glm::mat4(1.0f);
            model1 = glm::translate(model1, lights[i].position);
            model1 = glm::scale(model1, glm::vec3(0.2f));
            GameConstants::lightEmitterShader.setMat4("model", model1);
            GameConstants::lightEmitterShader.setVec3("lightColour", lights[i].diffuse);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // glDepthFunc(GL_LEQUAL);
        // glDepthMask(GL_FALSE);
        //
        // GameConstants::skyboxShader.use();
        //
        // float time = (float) glfwGetTime() * 0.5f - 0.0f;
        // GameConstants::skyboxShader.setFloat("time", time);
        //
        // GameConstants::skyboxShader.setMat4("view", view);
        // GameConstants::skyboxShader.setMat4("projection", projection);
        //
        // GameConstants::skyboxShader.setFloat("cirrus", 0.4f);
        // GameConstants::skyboxShader.setFloat("cumulus", 0.6f);
        //
        // skybox->drawBasic(GameConstants::skyboxShader);
        // glDepthMask(GL_TRUE);
        // glDepthFunc(GL_LESS);

        //disables gamma correction, so colours aren't washed out
        if (GameConstants::debugging) {
            glDisable(GL_FRAMEBUFFER_SRGB);
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glEnable(GL_FRAMEBUFFER_SRGB);
        }


        Profiler::endSection("Main");

        ImGui::EndFrame();

        Profiler::beginSection("GLFW Render");
        glfwSwapBuffers(window);
        glfwPollEvents();
        Profiler::endSection("GLFW Render");

        string title = ("Baseplate test game - Pos: " + to_string(GameConstants::player->position.x) + " " +
                        to_string(GameConstants::player->position.y) + " " +
                        to_string(GameConstants::player->position.z) + " Pitch: " +
                        to_string(GameConstants::player->pitch) + " Yaw: " + to_string(GameConstants::player->yaw));
        glfwSetWindowTitle(window, title.c_str());

        //here

        Profiler::beginSection("Sleep");
        float targetFrameTime = 1.0f / GameConstants::targetFPS;
        auto sleepDuration = targetFrameTime - deltaTime;
        if (sleepDuration > 0.002f) {
            std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::duration<float>(sleepDuration)
            ));
        }
        Profiler::endSection("Sleep");

        Profiler::endFrame();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    glfwSetWindowSize(window, width, height);
    GameConstants::window_width = width;
    GameConstants::window_height = height;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (!GameConstants::keybindsManager.TOGGLE_CURSOR->isPressd() && !GameConstants::debugging) {
        int windowX;
        int windowY;
        float maxSensitivity = 50.0f;
        float sensitivity = 50.0f;
        glfwGetWindowPos(window, &windowX, &windowY);

        //we dont wan NaN from /0
        GameConstants::player->yaw -= (xpos - (windowX + GameConstants::window_width / 2)) / max(maxSensitivity - sensitivity, 1.0f) /
                20;
        GameConstants::player->pitch -= (ypos - (windowY + GameConstants::window_height / 2)) / max(maxSensitivity - sensitivity, 1.0f)
                / 20;

        GameConstants::player->yaw = glm::mod(GameConstants::player->yaw, 360.0f);
        GameConstants::player->pitch = glm::clamp(GameConstants::player->pitch, -89.0f, 89.0f);

        glfwSetCursorPos(window, windowX + GameConstants::window_width / 2, windowY + GameConstants::window_height / 2);
    }
}
