//
// Created by dj on 30/06/2025.
//

#include "Keybind.h"

Keybind::Keybind(int key, Mode mode, std::string name, std::function<void()> pressed, std::function<void()> whilst,
                 std::function<void()> released, std::function<bool()> canBePressed) : pressed(pressed), whilst(whilst),
	released(released), canBePressed(canBePressed) {
	this->key = key;
	this->mode = mode;
	this->name = name;
}
