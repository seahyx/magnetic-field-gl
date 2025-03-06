#pragma once

#include <iostream>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <algorithm>
#include <vector>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <GLM\glm.hpp>
#include <GLM\gtc\matrix_transform.hpp>
#include <GLM\gtc\type_ptr.hpp>

#include "shader.h"

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

float vertices[] =
{
	//    x      y      z   
		-1.0f, -1.0f, -0.0f,
		 1.0f,  1.0f, -0.0f,
		-1.0f,  1.0f, -0.0f,
		 1.0f, -1.0f, -0.0f
};

unsigned int indices[] =
{
	//  2---,1
	//  | .' |
	//  0'---3
		0, 1, 2,
		0, 3, 1
};

/* Window resize callback function prototype */
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
/* Window input event callback function prototype */
void processInput(GLFWwindow* window);
/* FPS counter function prototype */
void countFPS();
/* Delta time updater function prototype */
void updateDeltaTime();