//
// Created by dj on 5/07/2025.
//

#pragma once
#include <string>
#include <chrono>
#include <unordered_map>
#include <iostream>
#include "GameConstants.h"

struct ProfileResult {
	float totalTime = 0.0f;
	int callCount = 0;
};

class Profiler {
public:
	static void beginFrame() {
		results.clear();
	}

	static void endFrame() {

	}

	static void beginSection(const std::string& name) {
		if (GameConstants::debug) startTimes[name] = std::chrono::high_resolution_clock::now();
	}

	static void endSection(const std::string& name) {
		if (GameConstants::debug) {
			auto now = std::chrono::high_resolution_clock::now();
			float ms = std::chrono::duration<float, std::milli>(now - startTimes[name]).count();
			results[name].totalTime += ms;
			results[name].callCount++;
		}
	}

	static const std::unordered_map<std::string, ProfileResult>& getResults() {
		return results;
	}

	static float fpsHistory[100];
	static int fpsIndex;

private:
	static inline std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> startTimes;
	static inline std::unordered_map<std::string, ProfileResult> results;
};
