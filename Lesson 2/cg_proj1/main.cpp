#include <iostream>

// 1. Include libraries
// Note: GLEW must always be included before GLFW
#define GLEW_STATIC // Because we are using glew32s.lib (Static)
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// --- 2. Shader Code ---

// A. Vertex Shader code
// Its job: define the position of triangle vertices in space
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n" // receive data at location 0
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n" // pass position as-is
"}\0";

// B. Fragment Shader code
// Its job: define the color of the pixels (here White)
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n" // RGBA: White
"}\n\0";

// Function to handle window resizing by the user
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Function to handle input (like pressing ESC)
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    // --- 3. Initialize GLFW and configure window ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // modern profile

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // required for Mac
#endif

    // Create window object
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL - First Triangle", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Make this window the current rendering context
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // Set resize callback

    // --- 4. Initialize GLEW ---
    glewExperimental = GL_TRUE; // Enable modern techniques
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // --- 5. Build and compile Shader Program ---

    // A. Compile Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // B. Compile Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check for errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // C. Link shaders into one Shader Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete individual shaders since they are linked into the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --- 6. Define data (rectangle made of two triangles) ---
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,

         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };

    // --- 7. Setup VBO and VAO ---
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO); // Generate IDs
    glGenBuffers(1, &VBO);

    // Very important: bind VAO first before anything else
    glBindVertexArray(VAO);

    // Bind VBO and copy data into it
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Tell OpenGL how to interpret vertex data (Linking Vertex Attributes)
    // 0: location in shader
    // 3: size of data (3 floats per vertex)
    // GL_FLOAT: data type
    // GL_FALSE: do not normalize data
    // 3 * sizeof(float): stride
    // (void*)0: offset (start of data)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Enable attribute 0

    // Unbind (optional but good practice)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --- 8. Render Loop ---
    while (!glfwWindowShouldClose(window))
    {
        // A. Process input
        processInput(window);

        // B. Clear screen (background color)
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f); //dark grey background
        glClear(GL_COLOR_BUFFER_BIT);

        // C. Draw
        glUseProgram(shaderProgram); // Activate shader
        glBindVertexArray(VAO); // Activate rectangle object
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw 6 vertices (2 triangles)

        // D. Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- 9. Final cleanup ---
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
