#pragma once

// std
#include <cstdint>
#include <functional>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

// entt
#include "libs/entt/entt.hpp"

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "libs/glm/glm.hpp"
#include "libs/glm/gtc/matrix_transform.hpp"
#include "libs/glm/gtc/type_ptr.hpp"
#include "libs/glm/gtx/matrix_decompose.hpp"
#include "libs/glm/gtx/quaternion.hpp"

namespace dank {
enum class ResourceState { Idle, Loading, Ready, Invalid };
enum class PixelFormat { RGBA8Unorm = 0, BGRA32 = 1 };
} // namespace dank
