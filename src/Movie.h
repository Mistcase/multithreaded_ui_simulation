#pragma once

#include "RenderContext.h"
#include "BackendContainerNode.h"
#include "BackendTextNode.h"
#include "FrontendNodes.h"
#include "TraceProfiler.h"
#include "OpenGLRenderer.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <vector>

namespace ui {

// Simple scene / Movie
class Movie {
public:
    Movie()
        : m_running(true)
        , m_rootId(RenderContext::Instance().AllocateNodeId())  // Allocated by RenderContext
        , m_rectId(RenderContext::Instance().AllocateNodeId()) {  // Allocated by RenderContext
        // Initialize OpenGL renderer
        if (!m_renderer.Initialize(800, 600, "UI Sandbox")) {
            std::cerr << "Failed to initialize OpenGL renderer" << std::endl;
        }

        // Frontend nodes create and own backend nodes
        m_root = FrontendContainer::Create(m_rootId);
        m_rect = FrontendShapeRect::Create(m_rectId);

        m_root->AddChild(m_rectId);

        m_root->SetPosition(0.0f, 0.0f);
        m_rect->SetPosition(10.0f, 20.0f);
        m_rect->SetWidth(100.0f);
        m_rect->SetHeight(50.0f);
    }

    ~Movie() {
        m_renderer.Shutdown();
    }

    void Stop() {
        m_running = false;
        m_renderer.Shutdown();
    }
    bool IsRunning() const { return m_running.load(); }

    // main_thread: update
    void Update() {
        TRACE_SCOPE("Movie::Update");
        SimulateScriptLanguageProcessing();
        
        if (!m_root)
            return;

        // Fake animation: move text along X axis
        static float x = 10.0f;
        static int frameCount = 0;
        frameCount++;
        x += 1.0f;
        
        if (m_rect)
        {
            m_rect->SetPosition(x, 20.0f);
        }
        
        // Example: delete text node after 3 frames
        if (frameCount == 3 && m_rect) {
            m_rect->Term();
            m_rect.reset();  // Clear frontend pointer
        }

        // At the end of update: sync buffer -> render tree
        RenderContext::Instance().Sync();
    }

    // render_thread: render
    void Render() {
        TRACE_SCOPE("Movie::Render");

        // Check if window should close (thread-safe check)
        if (m_renderer.ShouldClose()) {
            Stop();
            return;
        }

        {
            std::lock_guard<std::mutex> lock(RenderContext::Instance().RenderMutex());
            CollectRenderCommands();
        }

        // Execute render commands
        ExecuteRenderCommands();

        // Note: PollEvents() should be called from main thread on macOS
        // See ProcessEvents() method
    }

    // main_thread: process window events (must be called from main thread on macOS)
    void ProcessEvents() {
        m_renderer.PollEvents();

        // Check if window should close
        if (m_renderer.ShouldClose()) {
            Stop();
        }
    }

    void SimulateScriptLanguageProcessing() {
        TRACE_SCOPE("Movie::SimulateScriptLanguageProcessing");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

private:
    void CollectRenderCommands() {
        TRACE_SCOPE("Movie::CollectRenderCommands");
        auto& ctx = RenderContext::Instance();
        auto* rootRender = ctx.TryGetRenderNode<ContainerNodeData>(m_rootId);
        if (!rootRender) {
            return;
        }

        // Rebuild command buffer from scratch under render mutex to avoid
        // unsynchronized access later in ExecuteRenderCommands.
        m_renderCommands.clear();

        // DFS without map lookups: use already resolved child pointers
        std::vector<const RenderContainerNode*> stack;
        stack.push_back(rootRender);
        while (!stack.empty()) {
            const RenderContainerNode* node = stack.back();
            stack.pop_back();

            for (NodeId childId : node->children) {
                // Try container first (most common case for tree traversal)
                const RenderContainerNode* container = ctx.TryGetRenderNode<ContainerNodeData>(childId);
                if (container) {
                    stack.push_back(container);
                    continue;
                }

                // Try text node
                const RenderTextNode* text = ctx.TryGetRenderNode<TextNodeData>(childId);
                if (text && text->visible) {
                    RenderCommand cmd{};
                    cmd.type = RenderCommand::Type::Text;
                    cmd.textPayload.x = text->x;
                    cmd.textPayload.y = text->y;
                    cmd.textPayload.text = text->text;
                    m_renderCommands.push_back(std::move(cmd));
                }

                // Try shape rect node
                const RenderShapeRectNode* shapeRect = ctx.TryGetRenderNode<ShapeRectNodeData>(childId);
                if (shapeRect && shapeRect->visible) {
                    RenderCommand cmd{};
                    cmd.type = RenderCommand::Type::ShapeRect;
                    cmd.shapeRectPayload.x = shapeRect->x;
                    cmd.shapeRectPayload.y = shapeRect->y;
                    cmd.shapeRectPayload.width = shapeRect->width;
                    cmd.shapeRectPayload.height = shapeRect->height;
                    m_renderCommands.push_back(std::move(cmd));
                }

                // If all are nullptr: node was deleted, skip it
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }

    void ExecuteRenderCommands() {
        TRACE_SCOPE("Movie::ExecuteRenderCommands");

        m_renderer.BeginFrame();

        for (const auto& cmd : m_renderCommands) {
            switch (cmd.type) {
                case RenderCommand::Type::Text: {
                    m_renderer.RenderText(cmd.textPayload.x, cmd.textPayload.y, cmd.textPayload.text);
                    break;
                }
                case RenderCommand::Type::ShapeRect: {
                    if (cmd.shapeRectPayload.width > 0.0f && cmd.shapeRectPayload.height > 0.0f) {
                        // Use bright cyan color for visibility
                        m_renderer.RenderRect(
                            cmd.shapeRectPayload.x,
                            cmd.shapeRectPayload.y,
                            cmd.shapeRectPayload.width,
                            cmd.shapeRectPayload.height,
                            0.0f, 1.0f, 1.0f, 1.0f);
                    }
                    break;
                }
            }
        }

        m_renderer.EndFrame();
        m_renderCommands.clear();
    }

private:
    // Render command structure
    struct RenderCommand {
        enum class Type {
            Text,
            ShapeRect
        };
        Type type;
        struct TextPayload {
            float x = 0.0f;
            float y = 0.0f;
            std::string text;
        };
        struct ShapeRectPayload {
            float x = 0.0f;
            float y = 0.0f;
            float width = 0.0f;
            float height = 0.0f;
        };

        // Only one of the payloads is used depending on type.
        TextPayload textPayload;
        ShapeRectPayload shapeRectPayload;
    };

    std::atomic<bool> m_running;

    NodeId m_rootId;
    NodeId m_rectId;

    std::unique_ptr<FrontendContainer> m_root;
    std::unique_ptr<FrontendShapeRect> m_rect;

    OpenGLRenderer m_renderer;
    std::vector<RenderCommand> m_renderCommands;
};

} // namespace ui
