//
// Created by dj on 20/06/2025.
//

#pragma once
#include <string>

struct Vertex;
class CollisionPart;
class Model;

class ModelUtil {
public:
    static Model loadModelFromFile(const std::string &path);

    static void loadModels();

    static Model* getModel(const std::string &name);
};
