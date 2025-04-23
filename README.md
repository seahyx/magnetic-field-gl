# Magnetic Field Visualizer

This OpenGL-based application visualizes magnetic fields in real-time from user-manipulated dipoles in a 3D environment. Built with GLFW, GLM, ImGui, and custom shaders, it offers real-time interaction, field line tracing, and other nifty features.

## Setup and Build Instructions (Windows with Visual Studio)

### Prerequisites

- **Visual Studio**: Install Visual Studio 2019 or later (Community, Professional, or Enterprise) with the **Desktop development with C++** workload from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/).
- **GLFW**: Download 32-bit Windows binaries from [glfw.org/download.html](https://glfw.org/download.html). Copy `glfw3.lib` (e.g., from `lib-vc2019`) to the project’s `libs` folder.
- **Dependencies**: Ensure GLM, ImGui, and GLAD are in the `includes` folder. Shader files (`shader.frag`, `field_line.vert`, `field_line.frag`) should be in the project directory or a `shaders` subdirectory.
- **Directory Structure**:

  ```
  MagneticFieldVisualizer/
  ├── includes/         # GLM, ImGui, GLAD
  ├── libs/             # glfw3.lib
  ├── src/              # main.cpp, camera.cpp, etc.
  ├── shaders/          # shader files
  ├── MagneticFieldVisualizer.sln
  ├── README.md
  ```

### Visual Studio Setup

1. Open `MagneticFieldVisualizer.sln` in Visual Studio.
2. In **Project Properties**:
   - **C/C++ > General**: Add `$(SolutionDir)includes` to **Additional Include Directories**.
   - **Linker > General**: Add `$(SolutionDir)libs` to **Additional Library Directories**.
   - **Linker > Input**: Add `glfw3.lib;opengl32.lib` to **Additional Dependencies**.
   - Apply for **Debug** and **Release** configurations.
3. Verify source files (`main.cpp`, `camera.cpp`, etc.) are in **Source Files** and shaders are accessible.

### Build and Run

1. Set configuration to **Debug** or **Release** and platform to **Win64** (x64).
2. Click **Build > Build Solution** (`Ctrl+Shift+B`).
3. Run via **Debug > Start Without Debugging** (`Ctrl+F5`).
4. If errors occur:
   - Ensure `glfw3.lib` is in `libs`.
   - Check shader file paths in `main.cpp`.
   - Copy `glfw3.dll` to the executable directory if needed.

## Key Features

- **Dipole Interaction**: Add, move (Ctrl+drag), or rotate (Alt+drag) dipoles, visualized as spheres with directional arrows.
- **Field Visualization**: Real-time field plane with color gradients (blue to red) for field strength, adjustable via ImGui.
- **Field Line Tracing**: Traces field lines using a 4th-order Runge-Kutta method with adaptive steps, parallelized for performance.
- **Camera Controls**: Perspective/orthographic modes, zoom (scroll), rotate (drag), and reset. No more snapping, we promise!
- **ImGui Interface**: Controls camera, dipoles, field lines, and visualization settings, plus FPS display.

## Challenges

- **Camera Snapping**: Fixed by syncing pitch variables post-reset. Our camera now behaves like a well-trained pet.
- **Resource Management**: Robust VAO cleanup in `DipoleVisualizer` prevents OpenGL mishaps. Really this shit had me pulling out my hair and contemplate life.
- **Tracing Stability**: Adaptive Runge-Kutta ensures accurate field lines, even in tricky field regions.

## Credits

- **Field Line Tracing Algorithm**: Adapted from Chen, Jun’s 4th-order Runge-Kutta method in “Numerical solution of tracing magnetic field line” (2020), [el2718@mail.ustc.edu.cn](mailto:el2718@mail.ustc.edu.cn).
- **Dependencies**: GLFW, GLM, ImGui, GLAD. Thanks for carrying our code’s weight.
- **Team**: We stumbled through quaternions and shaders so you can enjoy pretty magnetic fields.

## License

Provided for educational use. Respect the licenses of GLFW, GLM, ImGui, and GLAD.
