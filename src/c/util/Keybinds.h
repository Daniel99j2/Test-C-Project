//
// Created by dj on 30/06/2025.
//

#pragma once

#include "GameConstants.h"
#include "../misc/Keybind.h"
#include "GenericUtil.h"
#include "../objects/type/Player.h"

class Keybinds {
public:
    std::vector<std::shared_ptr<Keybind>> keybinds;

    template<typename Pressed, typename Whilst, typename Released>
    std::shared_ptr<Keybind> make_keybind(int key, Keybind::Mode mode, std::string name, Pressed p, Whilst w,
                                          Released r) {
        auto shared = std::make_shared<Keybind>(key, mode, std::move(name), p, w, r);
        keybinds.push_back(shared);
        return shared;
    }

    std::shared_ptr<Keybind> MOVE_FORWARD = make_keybind(GLFW_KEY_W, Keybind::HOLD, "Move Forward",
                                                         []() {
                                                         },
                                                         []() {
                                                             GameConstants::player->move(
                                                                 true, false, false, false);
                                                         }, []() {
                                                         });
    std::shared_ptr<Keybind> MOVE_BACKWARD = make_keybind(GLFW_KEY_S, Keybind::HOLD, "Move Backward",
                                                          []() {
                                                          },
                                                          []() {
                                                              GameConstants::player->move(
                                                                  false, false, true, false);
                                                          }, []() {
                                                          });
    std::shared_ptr<Keybind> MOVE_LEFT = make_keybind(GLFW_KEY_A, Keybind::HOLD, "Move Left",
                                                      []() {
                                                      },
                                                      []() {
                                                          GameConstants::player->move(
                                                              false, true, false, false);
                                                      }, []() {
                                                      });
    std::shared_ptr<Keybind> MOVE_RIGHT = make_keybind(GLFW_KEY_D, Keybind::HOLD, "Move Right",
                                                       []() {
                                                       },
                                                       []() {
                                                           GameConstants::player->move(
                                                               false, false, false, true);
                                                       }, []() {
                                                       });
    std::shared_ptr<Keybind> MOVE_UP = make_keybind(GLFW_KEY_SPACE, Keybind::HOLD, "Move Up",
                                                    []() {
                                                    },
                                                    []() {
                                                        float max_speed = 10;
                                                        GameConstants::player->velocity.y += GameConstants::player->speed;

                                                        if (GameConstants::player->velocity.y > max_speed)
                                                            GameConstants::player->velocity.y = (glm::normalize(GameConstants::player->velocity) * max_speed).y;
                                                    }, []() {
                                                    });
    std::shared_ptr<Keybind> MOVE_DOWN = make_keybind(GLFW_KEY_LEFT_SHIFT, Keybind::HOLD, "Move Down",
                                                      []() {
                                                      },
                                                      []() {
                                                          float max_speed = 10;
                                                          GameConstants::player->velocity.y -= GameConstants::player->speed;

                                                          if (GameConstants::player->velocity.y < -max_speed)
                                                              GameConstants::player->velocity.y = (glm::normalize(GameConstants::player->velocity) * max_speed).y;
                                                      }, []() {
                                                      });
    std::shared_ptr<Keybind> TOGGLE_CAMERA = make_keybind(GLFW_KEY_C, Keybind::TOGGLE, "Toggle Camera",
                                                          []() {
                                                          },
                                                          []() {
                                                          }, []() {
                                                          });
    std::shared_ptr<Keybind> TOGGLE_CURSOR = make_keybind(GLFW_KEY_T, Keybind::TOGGLE, "Toggle Cursor lock",
                                                          []() {
                                                          },
                                                          []() {
                                                          }, []() {
                                                          });
    std::shared_ptr<Keybind> TOGGLE_DEBUGGING = make_keybind(GLFW_KEY_GRAVE_ACCENT, Keybind::TOGGLE, "Toggle Debugging",
                                                     []() {
                                                         if (GameConstants::debug) GameConstants::debugging = true;
                                                     },
                                                     []() {
                                                     }, []() {
                                                         if (GameConstants::debug) GameConstants::debugging = false;
                                                     });
    std::shared_ptr<Keybind> TOGGLE_FULLSCREEN = make_keybind(GLFW_KEY_F11, Keybind::TOGGLE, "Toggle Fullscreen",
                                                              [this]() {
                                                                  GameConstants::fullscreen = true;

                                                                  glfwGetWindowPos(
                                                                      GameConstants::window, &windowPosX, &windowPosY);
                                                                  glfwGetWindowSize(
                                                                      GameConstants::window, &windowedWidth,
                                                                      &windowedHeight);

                                                                  GLFWmonitor *monitor = GenericUtil::getCurrentMonitor(
                                                                      GameConstants::window);
                                                                  const GLFWvidmode *mode = glfwGetVideoMode(monitor);

                                                                  glfwSetWindowMonitor(
                                                                      GameConstants::window,
                                                                      monitor,
                                                                      0, 0,
                                                                      mode->width, mode->height,
                                                                      mode->refreshRate
                                                                  );

                                                                  GameConstants::window_width = mode->width;
                                                                  GameConstants::window_height = mode->height;
                                                              },
                                                              []() {
                                                              },
                                                              [this]() {
                                                                  GameConstants::fullscreen = false;

                                                                  glfwSetWindowMonitor(
                                                                      GameConstants::window,
                                                                      nullptr,
                                                                      windowPosX, windowPosY,
                                                                      windowedWidth, windowedHeight,
                                                                      GameConstants::targetFPS
                                                                  );

                                                                  GameConstants::window_width = windowedWidth;
                                                                  GameConstants::window_height = windowedHeight;
                                                              }
    );

private:
    int windowPosX, windowPosY;
    int windowedWidth, windowedHeight;
};
