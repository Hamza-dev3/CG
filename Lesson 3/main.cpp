
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Window size
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ==========================================
// Interaction state for the red triangle
// ==========================================
// redAlpha   : controls transparency of the red triangle (0.0 – 1.0)
// redXOffset : controls horizontal movement of the red triangle
float redAlpha = 0.6f;   // start semi-transparent
float redXOffset = 0.0f;   // start centered on X

// ==========================================
// Vertex Shader
// Original: position + color
// Now: also has a uniform "uXOffset" to move the red triangle left/right.
// ==========================================
const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"    // Input vertex position (X, Y, Z)
"layout (location = 1) in vec3 aColor;\n"  // Input vertex color (R, G, B)\n"
"out vec3 ourColor;\n"                     // Color passed to fragment shader\n"
"uniform float uXOffset;\n"                // Horizontal offset for the red triangle\n"
"void main()\n"
"{\n"
"    vec3 pos = aPos;\n"
"    pos.x += uXOffset;                    // Apply horizontal offset\n"
"    gl_Position = vec4(pos, 1.0);         // Final vertex position\n"
"    ourColor = aColor;                    // Pass the color along\n"
"}\0";

// ==========================================
// Fragment Shader
// Original: fixed color (orange).
// Now: uses interpolated color + uniform alpha "uAlpha" for transparency.
// ==========================================
const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;           // Color from vertex shader\n"
"uniform float uAlpha;       // Alpha from CPU (controls transparency)\n"
"void main()\n"
"{\n"
"    FragColor = vec4(ourColor, uAlpha);   // Final color with transparency\n"
"}\n\0";

// Callback when window/framebuffer is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Tell OpenGL the new drawing area
    glViewport(0, 0, width, height);
}

// ==========================================
// Input handling
// - ESC  : close window
// - SPACE: toggle wireframe / fill
// - W/S  : increase/decrease red triangle transparency
// - A/D  : move red triangle left/right
// ==========================================
void processInput(GLFWwindow* window)
{
    // Close window on ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Wireframe control using SPACE
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Line mode (wireframe)
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Fill mode (default)

    // W / S : modify redAlpha (transparency of red triangle)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        redAlpha += 0.01f;         // more opaque
        if (redAlpha > 1.0f)
            redAlpha = 1.0f;       // clamp to max
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        redAlpha -= 0.01f;         // more transparent
        if (redAlpha < 0.0f)
            redAlpha = 0.0f;       // clamp to min
    }

    // A / D : move red triangle on X axis
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        redXOffset -= 0.01f;       // move left

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        redXOffset += 0.01f;       // move right
}

int main()
{
    // ==========================================
    // Initialize GLFW and create a window
    // ==========================================
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);           // OpenGL version 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);           // OpenGL version x.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Core profile

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);     // Required on macOS
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "Lab 4: Advanced Control",
        nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Register the resize callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ==========================================
    // Initialize GLEW (must be done after context creation)
    // ==========================================
    glewExperimental = GL_TRUE; // Use modern function loading
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // ==========================================
    // Enable Advanced OpenGL Capabilities
    // - Depth testing: correct rendering by Z distance
    // - Blending    : support transparency (alpha)
    // ==========================================
    glEnable(GL_DEPTH_TEST);                                 // Enable depth testing
    glEnable(GL_BLEND);                                      // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);       // Blending equation

    // ==========================================
    // Build and compile shaders
    // ==========================================
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    // (You can add compile error checks here if you want)

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    // (Same: error checks can be added)

    // Link shaders into a shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Shaders are linked into the program; delete them
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ==========================================
    // Vertex Data: two triangles with color
    // Format per vertex: X, Y, Z, R, G, B
    // Red triangle: Z = 0.0 (closer)
    // Blue triangle: Z = 0.5 (farther)
    // ==========================================
    float vertices[] = {
        // First triangle (red - close, Z = 0.0)
        -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, // left-bottom
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, // right-bottom
         0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, // top

         // Second triangle (blue - far, Z = 0.5)
         -0.2f, -0.2f, 0.5f,   0.0f, 0.0f, 1.0f, // left-bottom
          0.8f, -0.2f, 0.5f,   0.0f, 0.0f, 1.0f, // right-bottom
          0.3f,  0.8f, 0.5f,   0.0f, 0.0f, 1.0f  // top
    };

    // ==========================================
    // VAO & VBO setup
    // VAO: stores the vertex attribute configuration
    // VBO: stores the actual vertex data on the GPU
    // ==========================================
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);                                 // Bind VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO);                     // Bind VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
        vertices, GL_STATIC_DRAW);                 // Copy vertex data

    // Position attribute (location = 0): 3 floats per vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (location = 1): 3 floats per vertex, after first 3 floats
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ==========================================
    // Render Loop
    // ==========================================
    while (!glfwWindowShouldClose(window))
    {
        // Handle keyboard input
        processInput(window);

        // Clear color and depth buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);              // Background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader program and VAO
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // Get uniform locations (for alpha and X offset)
        int alphaLoc = glGetUniformLocation(shaderProgram, "uAlpha");
        int xOffsetLoc = glGetUniformLocation(shaderProgram, "uXOffset");

        // ------------------------------------------
        // Draw blue triangle (far): fixed position, fully opaque
        // ------------------------------------------
        glUniform1f(xOffsetLoc, 0.0f);                     // No horizontal offset
        glUniform1f(alphaLoc, 1.0f);                       // Alpha = 1 (opaque)
        glDrawArrays(GL_TRIANGLES, 3, 3);                  // Start from vertex 3, 3 vertices

        // ------------------------------------------
        // Draw red triangle (close): controlled by keys
        // - redXOffset: A/D movement
        // - redAlpha  : W/S transparency
        // ------------------------------------------
        glUniform1f(xOffsetLoc, redXOffset);               // Apply horizontal offset
        glUniform1f(alphaLoc, redAlpha);                   // Apply alpha from keys
        glDrawArrays(GL_TRIANGLES, 0, 3);                  // Start from vertex 0, 3 vertices

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup GPU resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
