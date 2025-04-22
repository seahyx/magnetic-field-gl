#pragma once

#include <iostream>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <algorithm>
#include <vector>
#include <cmath>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "dipole.h"
#include "camera.h"
#include "cuboid.h"

constexpr auto PI = 3.141529;

constexpr auto PIXELS_PER_METER = 100.0f;

int screen_width{ 1080 };
int screen_height{ 1080 };

int num_frames{ 0 };
double last_time{ 0.0 };
double last_frame_time{ 0.0 };
double delta_time{ 0.0 };

float center_x{ 0.0f };
float center_y{ 0.0f };
float move_speed{ 5.0f };
double zoom{ 1.0 };
float zoom_speed{ 2.0f };
glm::dvec2 sel_pos = glm::dvec2(0.0);
float angle{ 0.0f };

/* Window resize callback function prototype */
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
/* Window input event callback function prototype */
void processInput(GLFWwindow* window);
/* FPS counter function prototype */
void countFPS();
/* Delta time updater function prototype */
void updateDeltaTime();