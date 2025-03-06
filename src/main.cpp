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
	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Ying Xiang's Mandelbrot Viewer", NULL, NULL);
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
	/* Register Callbacks */
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	

	
	// Setup vertex data and buffers and configure vertex attributes
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind VAO
	glBindVertexArray(VAO);

	// Bind VBO and copy vertices into buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize shader
	Shader our_shader("src/shader.vert", "src/shader.frag");

	// Initialize last_time for FPS counting
	last_time = glfwGetTime();
	updateDeltaTime();

	glEnable(GL_DEPTH_TEST);

	//============================================//
	//             START RENDER LOOP
	//============================================//
	while (!glfwWindowShouldClose(window))
	{
		// Process key inputs
		processInput(window);

		countFPS();
		updateDeltaTime();

		// Core rendering commands
		// Clear buffer with specified background color
		glClearColor(.2f, .3f, .3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		our_shader.use_shader();
		our_shader.set_float("zoom", zoom);
		our_shader.set_float("center_x", center_x);
		our_shader.set_float("center_y", center_y);
		our_shader.set_vec2("window", glm::vec2(screen_width, screen_height));

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Check and call events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//============================================//
	//              END RENDER LOOP
	//============================================//

	/* Cleanup */
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();

	return 0;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		center_y = center_y + move_speed * 0.1f * zoom * delta_time;
		if (center_y > 1.0f)
		{
			center_y = 1.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		center_y = center_y - move_speed * 0.1f * zoom * delta_time;
		if (center_y < -1.0f)
		{
			center_y = -1.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		center_x = center_x - move_speed * 0.1f * zoom * delta_time;
		if (center_x < -1.0f)
		{
			center_x = -1.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		center_x = center_x + move_speed * 0.1f * zoom * delta_time;
		if (center_x > 1.0f)
		{
			center_x = 1.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		zoom = zoom * (1.0f + zoom_speed * delta_time);
		if (zoom > 1.0f)
		{
			zoom = 1.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		zoom = zoom * (1.0f - zoom_speed * delta_time);
		if (zoom < 0.000001f)
		{
			zoom = 0.000001f;
		}
	}
}

void countFPS()
{
	double current_time = glfwGetTime();
	num_frames++;
	if (current_time - last_time >= 1.0f)
	{
		std::cout << 1000.0f / num_frames << "ms / frames" << std::endl;
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