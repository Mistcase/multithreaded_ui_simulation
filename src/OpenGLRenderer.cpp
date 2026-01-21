#include "OpenGLRenderer.h"

// Platform-specific OpenGL includes
// We try to keep include order and macros friendly for Windows/MSVC.

#ifdef _WIN32
// Ensure Windows types/macros (APIENTRY, WINGDIAPI, etc.) are defined
// before OpenGL headers on Windows.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#ifdef USE_GLFW
    #ifdef _WIN32
        // On Windows use GLEW to load modern OpenGL functions.
        // GLEW must be included before GLFW.
        #include <GL/glew.h>
    #endif

    #ifdef __APPLE__
        // On macOS, include OpenGL Core Profile headers before GLFW
        // This ensures all OpenGL functions are declared
        #define GL_SILENCE_DEPRECATION
        #include <OpenGL/gl3.h>
        #include <OpenGL/gl3ext.h>
    #endif
    // GLFW will include the appropriate GL headers for the platform
    #include <GLFW/glfw3.h>
#else
    // Fallback path without GLFW
    #ifdef __APPLE__
        #define GL_SILENCE_DEPRECATION
        #include <OpenGL/gl3.h>
        #include <OpenGL/gl3ext.h>
    #else
        // On Windows and other platforms without GLFW, use legacy GL headers.
        // On Windows this resolves to the Windows SDK GL headers.
        #include <GL/gl.h>
        #include <GL/glu.h>
    #endif
#endif

#include <iostream>
#include <vector>

namespace ui {

// Simple vertex shader for rendering rectangles
static const char* s_vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

uniform vec2 uScreenSize;

out vec4 FragColor;

void main() {
    // Convert from pixel coordinates to normalized device coordinates
    vec2 normalizedPos = vec2(
        (aPos.x / uScreenSize.x) * 2.0 - 1.0,
        1.0 - (aPos.y / uScreenSize.y) * 2.0
    );
    gl_Position = vec4(normalizedPos, 0.0, 1.0);
    FragColor = aColor;
}
)";

// Simple fragment shader
static const char* s_fragmentShaderSource = R"(
#version 330 core
in vec4 FragColor;
out vec4 color;

void main() {
    color = FragColor;
}
)";

OpenGLRenderer::OpenGLRenderer() {
}

OpenGLRenderer::~OpenGLRenderer() {
    Shutdown();
}

bool OpenGLRenderer::Initialize(int width, int height, const std::string& title) {
    if (m_initialized) {
        return true;
    }

    m_width = width;
    m_height = height;

#ifdef USE_GLFW
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    
    // Verify OpenGL context is current
    if (glfwGetCurrentContext() == nullptr) {
        std::cerr << "Failed to make OpenGL context current" << std::endl;
        Shutdown();
        return false;
    }
    
    // On macOS, verify OpenGL context is working
    // GLFW should handle function loading, but let's verify
    std::cout << "OpenGL context created successfully" << std::endl;
    
    glfwSwapInterval(1); // Enable VSync
#else
    // For macOS without GLFW, we'll need to create a context differently
    // This is a placeholder - in a real app you'd use NSOpenGLView or similar
    std::cout << "Warning: GLFW not available, using system OpenGL" << std::endl;
    // Note: Actual window creation would need platform-specific code
    return false; // Cannot proceed without GLFW on macOS
#endif

    // On Windows with GLEW, initialize the function loader now that we have a context.
#ifdef _WIN32
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: "
                  << reinterpret_cast<const char*>(glewGetErrorString(glewErr)) << std::endl;
        Shutdown();
        return false;
    }
#endif

    // Initialize shaders (desktop GL path)
    if (!InitializeShaders()) {
        std::cerr << "Failed to initialize shaders" << std::endl;
        Shutdown();
        return false;
    }

    // Setup OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    m_initialized = true;
    return true;
}

void OpenGLRenderer::Shutdown() {
    if (!m_initialized) {
        return;
    }

    CleanupShaders();

#ifdef USE_GLFW
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
#endif

    m_initialized = false;
}

bool OpenGLRenderer::InitializeShaders() {
    m_shaderProgram = CreateShaderProgram(s_vertexShaderSource, s_fragmentShaderSource);
    if (m_shaderProgram == 0) {
        return false;
    }

    // Create VAO and VBO for rectangle rendering
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
    // Setup vertex attributes
    // Position: 2 floats
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color: 4 floats
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return true;
}

void OpenGLRenderer::CleanupShaders() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
}

std::uint32_t OpenGLRenderer::CompileShader(const std::string& source, std::uint32_t type) {
    // Ensure context is current before calling OpenGL functions
#ifdef USE_GLFW
    if (m_window && glfwGetCurrentContext() != m_window) {
        glfwMakeContextCurrent(m_window);
    }
#endif

    std::uint32_t shader = glCreateShader(type);
    if (shader == 0) {
        std::cerr << "Failed to create shader. OpenGL context may not be valid." << std::endl;
        return 0;
    }
    
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Check compilation status
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

std::uint32_t OpenGLRenderer::CreateShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
    std::uint32_t vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        return 0;
    }

    std::uint32_t fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }

    std::uint32_t program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check linking status
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void OpenGLRenderer::BeginFrame() {
    if (!m_initialized) {
        return;
    }

    // Ensure context is current
#ifdef USE_GLFW
    if (m_window && glfwGetCurrentContext() != m_window) {
        glfwMakeContextCurrent(m_window);
    }
#endif

    glClear(GL_COLOR_BUFFER_BIT);

#ifndef _WIN32
    glUseProgram(m_shaderProgram);

    // Set screen size uniform
    int screenSizeLoc = glGetUniformLocation(m_shaderProgram, "uScreenSize");
    if (screenSizeLoc >= 0) {
        glUniform2f(screenSizeLoc, static_cast<float>(m_width), static_cast<float>(m_height));
    }
#endif
}

void OpenGLRenderer::EndFrame() {
    if (!m_initialized) {
        return;
    }

#ifdef USE_GLFW
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
#endif
}

void OpenGLRenderer::RenderRect(float x, float y, float width, float height, float r, float g, float b, float a) {
    if (!m_initialized) {
        return;
    }

    // Ensure context is current
#ifdef USE_GLFW
    if (m_window && glfwGetCurrentContext() != m_window) {
        glfwMakeContextCurrent(m_window);
    }
#endif

    // On Windows, use immediate mode for simplicity / compatibility.
#ifdef _WIN32
    glColor4f(r, g, b, a);

    glBegin(GL_TRIANGLES);
    // First triangle
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    // Second triangle
    glVertex2f(x, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
#else
    // Define rectangle vertices (2 triangles)
    // Each vertex: [x, y, r, g, b, a]
    float vertices[] = {
        x, y, r, g, b, a,                    // Top-left
        x + width, y, r, g, b, a,           // Top-right
        x + width, y + height, r, g, b, a,  // Bottom-right
        x, y + height, r, g, b, a           // Bottom-left
    };

    // Define indices for two triangles
    std::uint32_t indices[] = {
        0, 1, 2,  // First triangle
        0, 2, 3   // Second triangle
    };

    // Bind VAO first
    glBindVertexArray(m_VAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Create and bind EBO for indices (must be bound while VAO is bound)
    std::uint32_t EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Draw
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &EBO);
#endif
}

void OpenGLRenderer::RenderText(float x, float y, const std::string& text) {
    // Placeholder for text rendering
    // In a real implementation, you would use a font atlas or text rendering library
    // For now, render a simple rectangle as placeholder
    RenderRect(x, y, text.length() * 8.0f, 16.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}

bool OpenGLRenderer::ShouldClose() const {
#ifdef USE_GLFW
    if (m_window) {
        return glfwWindowShouldClose(m_window);
    }
#endif
    return false;
}

void OpenGLRenderer::PollEvents() {
#ifdef USE_GLFW
    if (m_window) {
        glfwPollEvents();
    }
#endif
}

} // namespace ui
