//
// Created by dj on 20/06/2025.
//

#pragma once
#include <iosfwd>
#include <string>
#include <vector>
#include <vector>

#include "Animation.h"

class Model;

class ModelUtil {
public:
    static void loadModels(bool forceRegen);

    static Model getModel(std::string name);

private:
    static Model genModel(const std::string& filePath);

    static void saveCBModel(const std::string &filepath, const Model &model);

    static Model loadCBModel(const std::string &filepath);
};
