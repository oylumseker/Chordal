#include "global.h"

std::random_device rand_dev;
std::mt19937 generator = std::mt19937(rand_dev());