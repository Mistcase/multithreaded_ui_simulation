#pragma once

#include "RenderContext.h"

#include <cstdint>
#include <string>

#ifdef USE_GLFW
#include <GLFW/glfw3.h>
#endif

namespace ui {

// OpenGL renderer for UI nodes
class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    // Initialize OpenGL context and resources
    bool Initialize(int width = 800, int height = 600, const std::string& title = "UI Sandbox");
    
    // Cleanup resources
    void Shutdown();

    // Begin/end frame rendering
    void BeginFrame();
    void EndFrame();

    // Render methods for different node types
    void RenderRect(float x, float y, float width, float height, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
    void RenderText(float x, float y, const std::string& text);
    
    // Check if window should close
    bool ShouldClose() const;
    
    // Get window dimensions
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    // Process window events (call in render loop)
    void PollEvents();

private:
    // Initialize shaders
    bool InitializeShaders();
    void CleanupShaders();
    
    // Compile shader from source
    std::uint32_t CompileShader(const std::string& source, std::uint32_t type);
    
    // Create shader program
    std::uint32_t CreateShaderProgram(const std::string& vertexSource, const std::string& fragmentSource);

#ifdef USE_GLFW
    GLFWwindow* m_window = nullptr;
#else
    void* m_window = nullptr; // Placeholder for future window implementation
#endif

    int m_width = 800;
    int m_height = 600;
    
    // OpenGL resources
    std::uint32_t m_shaderProgram = 0;
    std::uint32_t m_VAO = 0;
    std::uint32_t m_VBO = 0;
    bool m_initialized = false;
};

} // namespace ui
