#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Kinect.h>

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <limits>
#include <sstream>

#include "openvr_driver.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "pugixml.hpp"
