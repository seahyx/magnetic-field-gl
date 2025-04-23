#include "main.h"
#include <glm/gtx/string_cast.hpp>

void checkGLError(const std::string& context) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        if (context == "UBO update" && UBO_error_flagged) {
            continue;
			UBO_error_flagged = true;
        }
        std::cerr << "OpenGL error in " << context << ": " << err << std::endl;
    }
}

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    // Create window object
    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Magnetic Field Viewer", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Create Viewport
    glViewport(0, 0, screen_width, screen_height);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Initialize camera
    aspect_ratio = (float)screen_width / screen_height;
    main_camera.setPerspective(default_fov, aspect_ratio);
    main_camera.lookAt(glm::vec3(0.0f));

    // Initialize cuboid
    cuboid_height = cuboid_width / aspect_ratio;
    Cuboid cuboid(cuboid_width, cuboid_height, cuboid_depth);

    // Initialize field plane
    FieldPlane field_plane(cuboid_width, cuboid_height, field_plane_z);

    // Setup field rendering VAO
    unsigned int field_VAO, field_VBO, field_EBO;
    glGenVertexArrays(1, &field_VAO);
    glGenBuffers(1, &field_VBO);
    glGenBuffers(1, &field_EBO);

    glBindVertexArray(field_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, field_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(field_plane.vertices), field_plane.vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(field_plane.indices), field_plane.indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup cuboid edges VAO
    unsigned int cuboid_VAO, cuboid_VBO, cuboid_EBO;
    glGenVertexArrays(1, &cuboid_VAO);
    glGenBuffers(1, &cuboid_VBO);
    glGenBuffers(1, &cuboid_EBO);

    glBindVertexArray(cuboid_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cuboid_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cuboid.edge_vertices), cuboid.edge_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cuboid_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cuboid.edge_indices), cuboid.edge_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup field line VAO
    unsigned int field_line_VAO, field_line_VBO, field_line_EBO;
    glGenVertexArrays(1, &field_line_VAO);
    glGenBuffers(1, &field_line_VBO);
    glGenBuffers(1, &field_line_EBO);

    // Initialize shaders
    Shader field_shader("src/shader.vert", "src/shader.frag");
    Shader cuboid_shader("src/cuboid.vert", "src/cuboid.frag");
    Shader field_line_shader("src/field_line.vert", "src/field_line.frag");

    // Initialize UBO for dipoles
    unsigned int dipole_ubo;
    glGenBuffers(1, &dipole_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, dipole_ubo);
    const int max_dipoles = 1024;
    glBufferData(GL_UNIFORM_BUFFER, max_dipoles * sizeof(MagneticDipoleGL), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Bind UBO to binding point
    GLuint block_index = glGetUniformBlockIndex(field_shader.program_ID, "DipoleBuffer");
    if (block_index == GL_INVALID_INDEX) {
        std::cerr << "Failed to find uniform block 'DipoleBuffer'" << std::endl;
        glfwTerminate();
        return -1;
    }
    glUniformBlockBinding(field_shader.program_ID, block_index, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, dipole_ubo);

    // Reserve vector capacity to prevent reallocation
    dipoles.reserve(max_dipoles);
    dipole_visualizers.reserve(max_dipoles);

    // Initialize field line tracer
    std::vector<BaseMagnet*> magnets;
    for (auto& dipole : dipoles) {
        magnets.push_back(&dipole);
    }
    FieldLineTracer tracer(magnets, cuboid_width, cuboid_height, cuboid_depth,
        trace_step_size, trace_max_steps, trace_adaptive_min_step,
        trace_adaptive_max_step, trace_adaptive_field_ref,
        trace_use_adaptive_step, render_field_lines);
    std::vector<FieldLine> fieldLines;
    std::vector<float> field_line_vertices;
    std::vector<unsigned int> field_line_indices;
    bool field_lines_dirty = true; // Flag to indicate when field lines need updating

    // Initialize rendering variables
    float fov = default_fov;
    bool last_is_perspective = true;
    float moment = 0.1f;
    float current_pitch = 0.0f;

    // Variables for dipole dragging
    int selected_dipole_index = -1; // -1 means no dipole selected
    DragMode drag_mode = DragMode::None;
    glm::dvec2 drag_start_mouse_pos;
    glm::vec3 drag_start_position;
    glm::quat drag_start_rotation;

    // Enable depth testing and blending
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize last_time for FPS counting
    last_time = glfwGetTime();
    updateDeltaTime();

    // Start render loop
    while (!glfwWindowShouldClose(window))
    {
        // Process input
        processInput(window, selected_dipole_index, drag_mode, drag_start_mouse_pos, drag_start_position, drag_start_rotation);
        countFPS();
        updateDeltaTime();

        // Check if perspective mode toggled
        if (is_perspective != last_is_perspective)
        {
            if (is_perspective)
            {
                main_camera.setWorldPosition(saved_camera_position);
                main_camera.setWorldRotation(saved_camera_rotation);
                main_camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
            }
            else
            {
                saved_camera_position = main_camera.getWorldPosition();
                saved_camera_rotation = main_camera.getWorldRotation();
                main_camera.setWorldPosition(glm::vec3(0.0f, 0.0f, 5.0f));
                main_camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
                current_pitch = 0.0f;
            }
            last_is_perspective = is_perspective;
        }

        // Apply FOV update
        if (is_perspective)
        {
            main_camera.setPerspective(fov, aspect_ratio);
        }
        else
        {
            float ortho_size = cuboid_height / 2.0f;
            main_camera.setOrthographic(ortho_size, aspect_ratio);
        }

        // Update cuboid and field plane size
        if (screen_changed) {
            updateCuboidDimensions(cuboid, field_plane, cuboid_height, field_VAO, field_VBO, field_EBO, cuboid_VAO, cuboid_VBO, cuboid_EBO);
            tracer.updateBounds(cuboid_width, cuboid_height, cuboid_depth);
            field_lines_dirty = true;
        }

        // Update field plane geometry if z-position changed
        static float last_field_plane_z = field_plane_z;
        if (field_plane_z != last_field_plane_z) {
            field_plane.updateZPosition(field_plane_z, cuboid_depth);
            glBindBuffer(GL_ARRAY_BUFFER, field_VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(field_plane.vertices), field_plane.vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field_EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(field_plane.indices), field_plane.indices, GL_STATIC_DRAW);
            last_field_plane_z = field_plane_z;
        }

        // Handle dipole dragging
        if (selected_dipole_index >= 0 && drag_mode != DragMode::None)
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            glm::dvec2 current_mouse_pos(mouse_x, screen_height - mouse_y);

            if (drag_mode == DragMode::Move)
            {
                glm::vec2 normalized_coords = glm::vec2(
                    (2.0f * mouse_x) / screen_width - 1.0f,
                    1.0f - (2.0f * mouse_y) / screen_height
                );
                glm::vec4 ray_clip = glm::vec4(normalized_coords.x, normalized_coords.y, -1.0f, 1.0f);
                glm::vec4 ray_eye = glm::inverse(main_camera.getProjectionMatrix()) * ray_clip;
                ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
                glm::vec3 ray_world = glm::normalize(glm::vec3(glm::inverse(main_camera.getViewMatrix()) * ray_eye));
                glm::vec3 ray_origin = main_camera.getWorldPosition();

                glm::vec3 dipole_pos = dipoles[selected_dipole_index].getWorldPosition();
                glm::vec3 plane_normal = main_camera.getForward();
                float plane_d = -glm::dot(dipole_pos, plane_normal);

                float t = -(glm::dot(ray_origin, plane_normal) + plane_d) / glm::dot(ray_world, plane_normal);
                if (t > 0) {
                    glm::vec3 new_pos = ray_origin + t * ray_world;
                    dipoles[selected_dipole_index].setWorldPosition(new_pos);
                    field_lines_dirty = true;
                }
            }
            else if (drag_mode == DragMode::Rotate)
            {
                glm::dvec2 delta = current_mouse_pos - drag_start_mouse_pos;
                float rotate_sensitivity = 0.5f;
                float yaw = (float)delta.x * rotate_sensitivity;
                float pitch = (float)delta.y * rotate_sensitivity;

                glm::quat current_rotation = dipoles[selected_dipole_index].getWorldRotation();
                glm::quat rot_yaw = glm::angleAxis(glm::radians(yaw), main_camera.getUp());
                glm::quat rot_pitch = glm::angleAxis(glm::radians(-pitch), main_camera.getRight());
                glm::quat new_rotation = rot_yaw * rot_pitch * current_rotation;
                dipoles[selected_dipole_index].setWorldRotation(new_rotation);
                field_lines_dirty = true;

                drag_start_mouse_pos = current_mouse_pos;
            }
        }
        else if (is_perspective && is_dragging && !ImGui::GetIO().WantCaptureMouse && selected_dipole_index == -1)
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            glm::dvec2 current_mouse_pos(mouse_x, screen_height - mouse_y);
            glm::dvec2 delta = current_mouse_pos - last_mouse_pos;
            last_mouse_pos = current_mouse_pos;

            float yaw = delta.x * sensitivity * delta_time;
            float pitch = delta.y * sensitivity * delta_time;

            static float prev_pitch = current_pitch;
            current_pitch -= pitch;
            current_pitch = std::max(std::min(current_pitch, 89.0f), -89.0f);
            float pitch_delta = prev_pitch - current_pitch;
            prev_pitch = current_pitch;

            main_camera.rotateAround(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -yaw);
            glm::vec3 right_axis = main_camera.getRight();
            main_camera.rotateAround(glm::vec3(0.0f, 0.0f, 0.0f), right_axis, pitch_delta);
            main_camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
            saved_camera_position = main_camera.getWorldPosition();
            saved_camera_rotation = main_camera.getWorldRotation();
        }

        // Update field lines if necessary
        static bool last_trace_use_adaptive_step;
        static bool last_render_field_lines;
        if (field_lines_dirty || last_trace_use_adaptive_step != trace_use_adaptive_step || last_render_field_lines != render_field_lines) {
            if (render_field_lines) {
                magnets.clear();
                for (auto& dipole : dipoles) {
                    magnets.push_back(&dipole);
                }
                tracer.setTraceConfig(trace_step_size, trace_max_steps, trace_adaptive_min_step,
                    trace_adaptive_max_step, trace_adaptive_field_ref,
                    trace_use_adaptive_step, render_field_lines);
                fieldLines = tracer.traceFieldLines();
                updateFieldLineGeometry(fieldLines, field_line_VAO, field_line_VBO, field_line_EBO, field_line_vertices, field_line_indices);
            }
            field_lines_dirty = false;
            last_trace_use_adaptive_step = trace_use_adaptive_step;
            last_render_field_lines = render_field_lines;
        }

        // Clear screen
        glClearColor(.2f, .3f, .3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update UBO with dipole data
        std::vector<MagneticDipoleGL> dipoles_gl(dipoles.size());
        for (size_t i = 0; i < dipoles.size(); ++i) {
            dipoles_gl[i].position = dipoles[i].getWorldPosition();
            dipoles_gl[i].direction = dipoles[i].getDirection();
            dipoles_gl[i].moment = dipoles[i].getMoment();
        }
        glBindBuffer(GL_UNIFORM_BUFFER, dipole_ubo);
        glBufferData(GL_UNIFORM_BUFFER, max_dipoles * sizeof(MagneticDipoleGL), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, dipoles_gl.size() * sizeof(MagneticDipoleGL), dipoles_gl.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        checkGLError("UBO update");

        // Render dipole visualizers (opaque)
        for (auto& visualizer : dipole_visualizers) {
            visualizer.render(main_camera.getViewMatrix(), main_camera.getProjectionMatrix(), cuboid_shader);
        }

        // Render cuboid edges (opaque)
        cuboid_shader.use_shader();
        cuboid_shader.set_vec4("color", cuboid_color);
        cuboid_shader.set_mat4("view", main_camera.getViewMatrix());
        cuboid_shader.set_mat4("projection", main_camera.getProjectionMatrix());
        cuboid_shader.set_mat4("model", glm::mat4(1.0f));
        glBindVertexArray(cuboid_VAO);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); // Unbind VAO for safety

        // Render field lines (opaque)
        if (render_field_lines && !field_line_indices.empty()) {
            field_line_shader.use_shader();
            field_line_shader.set_mat4("view", main_camera.getViewMatrix());
            field_line_shader.set_mat4("projection", main_camera.getProjectionMatrix());
            field_line_shader.set_mat4("model", glm::mat4(1.0f));
            glBindVertexArray(field_line_VAO);
            glDrawElements(GL_LINES, field_line_indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Render magnetic field (transparent, no depth write)
        field_shader.use_shader();
        field_shader.set_mat4("view", main_camera.getViewMatrix());
        field_shader.set_mat4("projection", main_camera.getProjectionMatrix());
        field_shader.set_int("num_dipoles", (int)dipoles.size());
        field_shader.set_float("pixels_per_meter", PIXELS_PER_METER);
        field_shader.set_vec2("resolution", glm::vec2(screen_width, screen_height));
        field_shader.set_float("plane_opacity", field_plane_opacity);
        glDepthMask(GL_FALSE);
        glBindVertexArray(field_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Magnetic Field Visualiser");
        ImGui::Text("Camera Settings");
        ImGui::Text("FPS: %.1f", 1.0f / delta_time);
        ImGui::SliderFloat("FOV", &fov, 30.0f, 120.0f, "%.1f");
        ImGui::Checkbox("Perspective Mode", &is_perspective);

        ImGui::Separator();
        ImGui::Text("Field Plane Settings");
        ImGui::SliderFloat("Z Position", &field_plane_z, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Opacity", &field_plane_opacity, 0.0f, 1.0f, "%.2f");

        ImGui::Separator();
        ImGui::Text("Field Line Settings");
        ImGui::Checkbox("Render Field Lines", &render_field_lines);
        ImGui::Checkbox("Use Adaptive Step Size", &trace_use_adaptive_step);
        ImGui::SliderFloat("Step Size", &trace_step_size, 0.001f, 0.1f, "%.3f");
        ImGui::SliderInt("Max Steps", &trace_max_steps, 100, 2000);
        ImGui::SliderFloat("Adaptive Min Step", &trace_adaptive_min_step, 0.0001f, 0.01f, "%.4f");
        ImGui::SliderFloat("Adaptive Max Step", &trace_adaptive_max_step, 0.01f, 0.1f, "%.3f");
        ImGui::SliderFloat("Adaptive Field Ref", &trace_adaptive_field_ref, 0.01f, 1.0f, "%.2f");
        if (ImGui::Button("Apply Trace Settings")) {
            tracer.setTraceConfig(trace_step_size, trace_max_steps, trace_adaptive_min_step,
                trace_adaptive_max_step, trace_adaptive_field_ref,
                trace_use_adaptive_step, render_field_lines);
            field_lines_dirty = true;
        }
        if (ImGui::Button("Retrace Field Lines")) {
            field_lines_dirty = true;
        }

        ImGui::Separator();
        ImGui::Text("Dipole Management");

        if (ImGui::Button("Add Dipole")) {
            dipoles.emplace_back(
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                1.0f
            );
            dipole_visualizers.emplace_back(
                0.04f, 0.15f, &dipoles.back(),
                glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
            );
            dipole_visualizers.back().initialize();
            field_lines_dirty = true;
        }

        for (size_t i = 0; i < dipoles.size(); ++i) {
            ImGui::PushID(i);
            ImGui::Text("Dipole %d", i + 1);

            glm::vec3 pos = dipoles[i].getWorldPosition();
            float pos_array[3] = { pos.x, pos.y, pos.z };
            if (ImGui::InputFloat3("Position", pos_array)) {
                dipoles[i].setWorldPosition(glm::vec3(pos_array[0], pos_array[1], pos_array[2]));
                field_lines_dirty = true;
            }

            glm::vec3 euler = glm::degrees(glm::eulerAngles(dipoles[i].getWorldRotation()));
            float euler_array[3] = { euler.x, euler.y, euler.z };
            if (ImGui::InputFloat3("Rotation (Euler)", euler_array)) {
                dipoles[i].setWorldRotationEuler(glm::vec3(euler_array[0], euler_array[1], euler_array[2]));
                field_lines_dirty = true;
            }

            float moment = dipoles[i].getMoment();
            if (ImGui::InputFloat("Moment", &moment)) {
                dipoles[i].setMoment(moment);
                field_lines_dirty = true;
            }

            if (ImGui::Button("Remove Dipole")) {
                if (dipoles.size() != dipole_visualizers.size()) {
                    std::cerr << "Error: dipoles and dipole_visualizers sizes mismatch!" << std::endl;
                }
                else {
                    std::cout << "Removing dipole " << i << ": pos=" << glm::to_string(dipoles[i].getWorldPosition())
                        << ", moment=" << dipoles[i].getMoment() << std::endl;
                    if (i < dipoles.size() - 1) {
                        std::swap(dipoles[i], dipoles.back());
                        std::swap(dipole_visualizers[i], dipole_visualizers.back());
                        dipole_visualizers[i].setParent(&dipoles[i]);
                    }
                    dipoles.pop_back();
                    dipole_visualizers.pop_back();
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    glBindVertexArray(0);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                    checkGLError("After dipole removal");
                }
                if (selected_dipole_index == i) {
                    selected_dipole_index = -1;
                    drag_mode = DragMode::None;
                }
                else if (selected_dipole_index > i) {
                    --selected_dipole_index;
                }
                --i;
                field_lines_dirty = true;
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &field_VAO);
    glDeleteBuffers(1, &field_VBO);
    glDeleteBuffers(1, &field_EBO);
    glDeleteVertexArrays(1, &cuboid_VAO);
    glDeleteBuffers(1, &cuboid_VBO);
    glDeleteBuffers(1, &cuboid_EBO);
    glDeleteVertexArrays(1, &field_line_VAO);
    glDeleteBuffers(1, &field_line_VBO);
    glDeleteBuffers(1, &field_line_EBO);
    glDeleteBuffers(1, &dipole_ubo);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

/* Window resize callback function */
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    screen_changed = width != screen_width || height != screen_height;
    screen_width = width;
    screen_height = height;
    glViewport(0, 0, width, height);

    // Update aspect ratio and cuboid height
    aspect_ratio = (float)screen_width / screen_height;
    cuboid_height = cuboid_width / aspect_ratio;
}

/* Scroll callback function */
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (is_perspective && !ImGui::GetIO().WantCaptureMouse)
    {
        // Calculate zoom delta
        float zoom_delta = -yoffset * zoom_speed; // Negative because scrolling up (positive yoffset) zooms in
        glm::vec3 current_pos = main_camera.getWorldPosition();
        float distance = glm::length(current_pos); // Distance from origin (0, 0, 0)
        float new_distance = distance + zoom_delta;

        // Clamp distance
        new_distance = std::max(std::min(new_distance, max_zoom), min_zoom);

        // Scale position to new distance
        glm::vec3 direction = glm::normalize(current_pos);
        glm::vec3 new_pos = direction * new_distance;
        main_camera.setWorldPosition(new_pos);
        main_camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        // Update saved position
        saved_camera_position = new_pos;
    }
}

/* Window input event callback function */
void processInput(GLFWwindow* window, int& selected_dipole_index, DragMode& drag_mode, glm::dvec2& drag_start_mouse_pos, glm::vec3& drag_start_position, glm::quat& drag_start_rotation)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Handle mouse button for dragging
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
    {
        if (!is_dragging)
        {
            is_dragging = true;
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            last_mouse_pos = glm::dvec2(mouse_x, screen_height - mouse_y);
            drag_start_mouse_pos = last_mouse_pos;

            // Raycasting for dipole selection
            glm::vec2 normalized_coords = glm::vec2(
                (2.0f * mouse_x) / screen_width - 1.0f,
                1.0f - (2.0f * mouse_y) / screen_height
            );
            glm::vec4 ray_clip = glm::vec4(normalized_coords.x, normalized_coords.y, -1.0f, 1.0f);
            glm::vec4 ray_eye = glm::inverse(main_camera.getProjectionMatrix()) * ray_clip;
            ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
            glm::vec3 ray_world = glm::normalize(glm::vec3(glm::inverse(main_camera.getViewMatrix()) * ray_eye));
            glm::vec3 ray_origin = main_camera.getWorldPosition();

            selected_dipole_index = -1;
            float closest_dist = std::numeric_limits<float>::max();
            const float sphere_radius = 0.04f; // From DipoleVisualizer

            for (size_t i = 0; i < dipoles.size(); ++i)
            {
                glm::vec3 dipole_pos = dipoles[i].getWorldPosition();
                float dist;
                if (glm::intersectRaySphere(ray_origin, ray_world, dipole_pos, sphere_radius * sphere_radius, dist))
                {
                    if (dist < closest_dist)
                    {
                        closest_dist = dist;
                        selected_dipole_index = i;
                    }
                }
            }

            if (selected_dipole_index >= 0)
            {
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
                {
                    drag_mode = DragMode::Move;
                    drag_start_position = dipoles[selected_dipole_index].getWorldPosition();
                }
                else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
                {
                    drag_mode = DragMode::Rotate;
                    drag_start_rotation = dipoles[selected_dipole_index].getWorldRotation();
                }
                else
                {
                    drag_mode = DragMode::None;
                    selected_dipole_index = -1;
                }
            }
        }
    }
    else
    {
        if (is_dragging)
        {
            is_dragging = false;
            selected_dipole_index = -1;
            drag_mode = DragMode::None;
        }
    }
}

/* FPS counter function */
void countFPS()
{
    double current_time = glfwGetTime();
    num_frames++;
    if (current_time - last_time >= 1.0f)
    {
        std::cout << "FPS: " << num_frames << ", " << 1000.0 * (current_time - last_time) / num_frames << "ms / frame" << std::endl;
        num_frames = 0;
        last_time += 1.0f;
    }
}

/* Delta time updater function */
void updateDeltaTime()
{
    double current_time = glfwGetTime();
    delta_time = current_time - last_frame_time;
    last_frame_time = current_time;
}

/* Cuboid dimension updater function */
void updateCuboidDimensions(Cuboid& cuboid, FieldPlane& field_plane, float cuboid_height, unsigned int field_VAO, unsigned int field_VBO, unsigned int field_EBO, unsigned int cuboid_VAO, unsigned int cuboid_VBO, unsigned int cuboid_EBO)
{
    // Update cuboid geometry
    cuboid.updateDimensions(cuboid_width, cuboid_height, cuboid_depth);

    // Update field plane geometry
    field_plane.updateDimensions(cuboid_width, cuboid_height, cuboid_depth);

    // Update field VBO and EBO
    glBindBuffer(GL_ARRAY_BUFFER, field_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(field_plane.vertices), field_plane.vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(field_plane.indices), field_plane.indices, GL_STATIC_DRAW);

    // Update cuboid edges VBO and EBO
    glBindBuffer(GL_ARRAY_BUFFER, cuboid_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cuboid.edge_vertices), cuboid.edge_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cuboid_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cuboid.edge_indices), cuboid.edge_indices, GL_STATIC_DRAW);

    // Rebind VAOs and re-specify vertex attributes to ensure state consistency
    glBindVertexArray(field_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, field_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field_EBO);
    glBindVertexArray(0);

    glBindVertexArray(cuboid_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cuboid_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cuboid_EBO);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Function to update field line geometry
void updateFieldLineGeometry(const std::vector<FieldLine>& fieldLines, unsigned int& field_line_VAO, unsigned int& field_line_VBO, unsigned int& field_line_EBO, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    vertices.clear();
    indices.clear();
    unsigned int vertexOffset = 0;

    for (const auto& line : fieldLines) {
        if (line.points.size() < 2) continue; // Need at least two points for a line segment
        for (size_t i = 0; i < line.points.size(); ++i) {
            const auto& point = line.points[i];
            // Vertex: position (x, y, z), field (x, y, z)
            vertices.push_back(point.position.x);
            vertices.push_back(point.position.y);
            vertices.push_back(point.position.z);
            vertices.push_back(point.field.x);
            vertices.push_back(point.field.y);
            vertices.push_back(point.field.z);

            // Indices for line segments
            if (i < line.points.size() - 1) {
                indices.push_back(vertexOffset + i);
                indices.push_back(vertexOffset + i + 1);
            }
        }
        vertexOffset += line.points.size();
    }

    // Update VBO
    glBindBuffer(GL_ARRAY_BUFFER, field_line_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Update EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field_line_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Configure VAO
    glBindVertexArray(field_line_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, field_line_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Field
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field_line_EBO);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}