#include "main.h"

int main()
{
    /* Initialize GLFW */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

    /* Initialize GLAD */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Create Viewport */
    glViewport(0, 0, screen_width, screen_height);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Setup vertex data and buffers
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Initialize variables
    float moment = 0.1f;

    // Initialize shader
    Shader our_shader("src/shader.vert", "src/shader.frag");

    // Initialize magnetic dipoles
    std::vector<MagneticDipole> dipoles;
    float horizontal_spacing = 200.0f;
    float vertical_spacing = 200.0f;
    for (int i = 0; i < 10; i++)
    {
        dipoles.push_back(MagneticDipole(glm::vec2(horizontal_spacing, vertical_spacing), glm::vec2(0.0f, 1.0f), moment));
        vertical_spacing += 100.0f;
        if (i == 4)
        {
            horizontal_spacing += 100.0f;
            vertical_spacing = 200.0f;
        }
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Start render loop
    while (!glfwWindowShouldClose(window))
    {
        // Process input
        processInput(window);

        // Clear screen
        glClearColor(.2f, .3f, .3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Use shader
        our_shader.use_shader();
        our_shader.set_vec2("window", glm::vec2(screen_width, screen_height));

        // Pass dipole data to the shader
        our_shader.set_int("num_dipoles", (int)dipoles.size());
        for (int i = 0; i < dipoles.size(); i++)
        {
            // renderFieldLines(dipoles[i], VAO, VBO);
            std::string base = "dipoles[" + std::to_string(i) + "]";
            our_shader.set_vec2(base + ".position", dipoles[i].getPosition());
            our_shader.set_vec2(base + ".direction", dipoles[i].getDirection());
            our_shader.set_float(base + ".moment", dipoles[i].getMoment());
        }

        // Update the first dipole's position dynamically based on mouse input
        glfwGetCursorPos(window, &sel_pos.x, &sel_pos.y);
        sel_pos.y = screen_height - sel_pos.y; // Adjust for OpenGL coordinate system
        dipoles[0].setPosition(sel_pos);

        // Render scene
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
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