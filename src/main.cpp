#include "main.h"

int main()
{
    /*
    * Initialize GLFW
    *
    * GLFW is configured using glfwWindowHint.
    * glfwWindowHint takes in 2 parameters, the option, and the value.
    * option is selected from a large enum of possible options prefixed with 'GLFW_'.
    * value is an integer that sets the value of the option.
    *
    * All options can be found here: https://www.glfw.org/docs/latest/window.html#window_hints
    */
    glfwInit();
    // Set the target OpenGL version to 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Use core-profile mode without backwards-compatible features
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create window object */
    GLFWwindow *window = glfwCreateWindow(screen_width, screen_height, "Magnetic Field Viewer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    /*
    * Initialize GLAD
    *
    * GLAD manages function pointers for OpenGL, so we have to initialize it before
    * calling any OpenGL function.
    *
    * GLFW defines glfwGetProcAddress for us, which is an OS-specific address of the
    * OpenGL function pointers.
    */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Create Viewport */
    glViewport(0, 0, screen_width, screen_height);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    /* Setup Dear ImGui context */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Initialize camera
    float aspect_ratio = (float)screen_width / screen_height;
    Camera camera(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f), 0.1f, 100.0f);
    camera.setPerspective(60.0f, aspect_ratio);

    // Initialize cuboid (4x3x2 for aspect ratio matching window)
    float cuboid_width = 4.0f;
    float cuboid_height = cuboid_width / aspect_ratio;
    float cuboid_depth = 2.0f;
    Cuboid cuboid(cuboid_width, cuboid_height, cuboid_depth);

    // Setup field rendering VAO
    unsigned int field_VAO, field_VBO, field_EBO;
    glGenVertexArrays(1, &field_VAO);
    glGenBuffers(1, &field_VBO);
    glGenBuffers(1, &field_EBO);

    glBindVertexArray(field_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, field_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cuboid.field_vertices), cuboid.field_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cuboid.field_indices), cuboid.field_indices, GL_STATIC_DRAW);
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

    // Initialize shaders
    Shader field_shader("src/shader.vert", "src/shader.frag");
    Shader cuboid_shader("src/cuboid.vert", "src/cuboid.frag");

    // Initialize magnetic dipoles
    std::vector<MagneticDipole> dipoles;
    dipoles.push_back(MagneticDipole(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.1f));

    // Initialize variables
    float fov = 60.0f;
    bool is_perspective = true;
    float moment = 0.1f;

    // Enable depth testing and blending
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize last_time for FPS counting
    last_time = glfwGetTime();
    updateDeltaTime();

    // Start render loop
    while (!glfwWindowShouldClose(window))
    {
        // Process input
        processInput(window);
        countFPS();
        updateDeltaTime();

        // Update camera projection based on ImGui FOV
        camera.setPerspective(fov, aspect_ratio);
        if (!is_perspective) {
            camera.setOrthographic(cuboid_height / 2.0f, aspect_ratio);
            camera.setWorldPosition(glm::vec3(0.0f, 5.0f, 0.0f));
            camera.lookAt(glm::vec3(0.0f));
        }

        // Handle mouse dragging for rotation in perspective mode
        if (is_perspective && is_dragging) {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            glm::dvec2 current_mouse_pos(mouse_x, screen_height - mouse_y);
            glm::dvec2 delta = current_mouse_pos - last_mouse_pos;
            last_mouse_pos = current_mouse_pos;

            float sensitivity = 0.5f;
            float yaw = delta.x * sensitivity * delta_time;
            float pitch = delta.y * sensitivity * delta_time;

            glm::vec3 pos = camera.getWorldPosition();
            glm::quat rot_yaw = glm::angleAxis(glm::radians(-yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat rot_pitch = glm::angleAxis(glm::radians(-pitch), camera.getRight());
            glm::quat rot = rot_yaw * rot_pitch;
            pos = rot * pos;
            camera.setWorldPosition(pos);
            camera.lookAt(glm::vec3(0.0f));
        }

        // Clear screen
        glClearColor(.2f, .3f, .3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render magnetic field
        field_shader.use_shader();
        field_shader.set_mat4("view", camera.getViewMatrix());
        field_shader.set_mat4("projection", camera.getProjectionMatrix());
        field_shader.set_int("num_dipoles", (int)dipoles.size());
        field_shader.set_float("pixels_per_meter", PIXELS_PER_METER);
        for (int i = 0; i < dipoles.size(); i++)
        {
            std::string base = "dipoles[" + std::to_string(i) + "]";
            field_shader.set_vec3(base + ".position", dipoles[i].getWorldPosition());
            field_shader.set_vec3(base + ".direction", dipoles[i].getDirection());
            field_shader.set_float(base + ".moment", dipoles[i].getMoment());
        }
        glBindVertexArray(field_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Render cuboid edges
        cuboid_shader.use_shader();
        cuboid_shader.set_mat4("view", camera.getViewMatrix());
        cuboid_shader.set_mat4("projection", camera.getProjectionMatrix());
        glBindVertexArray(cuboid_VAO);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

        // ImGui UI
        ImGui::Begin("UI");
        ImGui::Text("Adjust Moving Dipole Moment");
        ImGui::SliderFloat("Dipole Moment", &moment, 0.5f, 2.0f);
        for (int i = 0; i < dipoles.size(); i++)
        {
            dipoles[i].setMoment(moment); // Update the moment of the first dipole
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
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    screen_width = width;
    screen_height = height;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        angle += 120.0f * delta_time;
        if (angle >= 360.0f)
            angle -= 360.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        angle -= 120.0f * delta_time;
        if (angle < 0.0f)
            angle += 360.0f;
    }
}

void countFPS()
{
    double current_time = glfwGetTime();
    num_frames++;
    if (current_time - last_time >= 1.0f)
    {
        std::cout << 1000.0 * (current_time - last_time) / num_frames << "ms / frame" << std::endl;
        num_frames = 0;
        last_time += 1.0f;
    }
}

void updateDeltaTime()
{
    double current_time = glfwGetTime();
    delta_time = current_time - last_frame_time;
    last_frame_time = current_time;
}