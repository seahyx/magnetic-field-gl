#pragma once

#include <iostream>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <algorithm>
#include <vector>
#include <cmath>
#include <deque>
#include <random>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

#include "shader.h"
#include "shaders.h"
#include "dipole.h"
#include "camera.h"
#include "cuboid.h"
#include "dipole_visualizer.h"
#include "field_plane.h"
#include "field_line_tracer.h"

// Constants
constexpr auto PI = 3.141529;
constexpr auto PIXELS_PER_METER = 100.0f;

// Magnetic dipole struct to be used in UBO, matches struct in shader.frag
struct MagneticDipoleGL {
    glm::vec3 position;  // 12 bytes
    float padding1;      // 4 bytes padding to 16 bytes
    glm::vec3 direction; // 12 bytes
    float moment;        // 4 bytes supposed to be padding but this works for some reason idk why don't fkin ask
};

// Struct to store dipole state for history
struct DipoleState {
    glm::vec3 position;
    glm::quat rotation;
    float moment;
};

// Window settings
int screen_width{ 1080 };              // Default window width
int screen_height{ 1080 };             // Default window height
bool screen_changed{ false };          // Flag indicating if window size changed
float cuboid_height{ 0.0f };           // Height of the cuboid, adjusted by aspect ratio
float aspect_ratio{ 0.0f };            // Window aspect ratio (width / height)

// Camera settings
constexpr float nearPlane = 0.1f;      // Near clipping plane
constexpr float farPlane = 50.0f;      // Far clipping plane
constexpr float default_fov = 60.0f;   // Default field of view
constexpr float sensitivity = 10.0f;   // Mouse sensitivity for rotation
constexpr float zoom_speed = 0.5f;     // Zoom speed for scroll
constexpr float min_zoom = 1.0f;       // Minimum zoom distance
constexpr float max_zoom = 20.0f;      // Maximum zoom distance
Camera main_camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), nearPlane, farPlane); // Main camera
glm::vec3 saved_camera_position = main_camera.getWorldPosition(); // Saved camera position for perspective toggle
glm::quat saved_camera_rotation = main_camera.getWorldRotation(); // Saved camera rotation for perspective toggle
glm::vec3 look_at_point = glm::vec3(0.0f); // Camera look-at point (center of rotation)

// Cuboid settings
constexpr float cuboid_width = 4.0f;   // Cuboid width
constexpr float cuboid_depth = 4.0f;   // Cuboid depth
glm::vec4 cuboid_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // White color for cuboid edges

// Dipole settings
std::vector<MagneticDipole> dipoles;   // Collection of magnetic dipoles
std::vector<DipoleVisualizer> dipole_visualizers; // Visualizers for magnetic dipoles

// Field plane settings
float field_plane_z = 0.0f;            // Z-position in [-1, 1]
float field_plane_opacity = 0.5f;      // Opacity in [0, 1]
float field_plane_sensitivity = 1.0f;  // Sensitivity, to be multiplied to a power of 10

// Label settings
bool show_labels = true; // Added for label visibility toggle
bool show_labels_on_hover = false; // Added for hover-based label display

// Simulation settings
bool simulate = false; // Whether simulation is running
float simulation_speed = 1.0f; // Simulation time speed (seconds)
bool reverse_time = false; // Whether to run simulation backward

// Enum for dipole dragging modes
enum class DragMode { None, Move, Rotate, CameraDrag }; // Added CameraDrag

// Magnetic field line tracing settings
float trace_step_size = 0.01f; // Fixed step size in pixels
int trace_max_steps = 1000;   // Maximum number of steps per trace
float trace_adaptive_min_step = 0.001f; // Minimum step size for adaptive method
float trace_adaptive_max_step = 0.05f;  // Maximum step size for adaptive method
float trace_adaptive_field_ref = 0.1f;  // Reference field strength for adaptive scaling
bool trace_use_adaptive_step = false;   // Flag to switch between fixed and adaptive step size
bool render_field_lines = true;         // Flag to enable/disable field line rendering

// Timing and input variables
int num_frames{ 0 };                   // Frame counter for FPS calculation
double last_time{ 0.0 };               // Last time FPS was calculated
double last_frame_time{ 0.0 };         // Time of last frame
double delta_time{ 0.0 };              // Time between frames
bool is_dragging{ false };             // Flag indicating if mouse is dragging
bool is_perspective{ true };           // Flag for perspective vs orthographic mode
glm::dvec2 last_mouse_pos = glm::dvec2(0.0); // Last mouse position for dragging

// Errors and shit
bool UBO_error_flagged = false;

/* Window resize callback function prototype */
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
/* Window input event callback function prototype */
void processInput(GLFWwindow* window, int& selected_dipole_index, enum class DragMode& drag_mode, glm::dvec2& drag_start_mouse_pos, glm::vec3& drag_start_position, glm::quat& drag_start_rotation);
/* FPS counter function prototype */
void countFPS();
/* Delta time updater function prototype */
void updateDeltaTime();
/* Scroll callback function prototype */
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
/* Cuboid dimension updater function prototype */
void updateCuboidDimensions(Cuboid& cuboid, FieldPlane& field_plane, float cuboid_height, unsigned int field_VAO, unsigned int field_VBO, unsigned int field_EBO, unsigned int cuboid_VAO, unsigned int cuboid_VBO, unsigned int cuboid_EBO);
/* Field line geometry updater function prototype */
void updateFieldLineGeometry(const std::vector<FieldLine>& fieldLines, unsigned int& field_line_VAO, unsigned int& field_line_VBO, unsigned int& field_line_EBO, std::vector<float>& vertices, std::vector<unsigned int>& indices);