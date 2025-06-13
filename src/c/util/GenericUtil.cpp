#include <iostream>
#include <chrono>
#include <random>
#include "GenericUtil.h"
using namespace std;

int GenericUtil::randomInt(int min, int max) {
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    minstd_rand0 generator (seed);
    return min + generator() % (max - min + 1);
}