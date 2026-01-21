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

        m_root->AddChild(m_rect.get());

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

        auto collectLeafCommands = [&](RenderTextNode* text) -> const std::vector<RenderCommand>& {
            if (!text->isCommandsCacheValid) {
                text->cachedCommands.clear();
                RenderCommand cmd{};
                cmd.type = RenderCommand::Type::Text;
                cmd.textPayload.x = text->x;
                cmd.textPayload.y = text->y;
                cmd.textPayload.text = text->text;
                text->cachedCommands.push_back(std::move(cmd));
                text->isCommandsCacheValid = true;
            }
            return text->cachedCommands;
        };

        auto collectShapeRectCommands = [&](RenderShapeRectNode* rect) -> const std::vector<RenderCommand>& {
            if (!rect->isCommandsCacheValid) {
                rect->cachedCommands.clear();
                RenderCommand cmd{};
                cmd.type = RenderCommand::Type::ShapeRect;
                cmd.shapeRectPayload.x = rect->x;
                cmd.shapeRectPayload.y = rect->y;
                cmd.shapeRectPayload.width = rect->width;
                cmd.shapeRectPayload.height = rect->height;
                rect->cachedCommands.push_back(std::move(cmd));
                rect->isCommandsCacheValid = true;
            }
            return rect->cachedCommands;
        };

        std::function<void(RenderContainerNode*, std::vector<RenderCommand>&)> collectFromContainer =
            [&](RenderContainerNode* node, std::vector<RenderCommand>& out) {
                if (node->isCommandsCacheValid) {
                    out.insert(out.end(), node->cachedCommands.begin(), node->cachedCommands.end());
                    return;
                }

                node->cachedCommands.clear();
                for (NodeId childId : node->children) {
                    if (auto* container = ctx.TryGetRenderNode<ContainerNodeData>(childId)) {
                        collectFromContainer(container, node->cachedCommands);
                        continue;
                    }

                    if (auto* text = ctx.TryGetRenderNode<TextNodeData>(childId)) {
                        if (text->visible) {
                            const auto& cmds = collectLeafCommands(text);
                            node->cachedCommands.insert(node->cachedCommands.end(), cmds.begin(), cmds.end());
                        }
                        continue;
                    }

                    if (auto* shapeRect = ctx.TryGetRenderNode<ShapeRectNodeData>(childId)) {
                        if (shapeRect->visible && shapeRect->width > 0.0f && shapeRect->height > 0.0f) {
                            const auto& cmds = collectShapeRectCommands(shapeRect);
                            node->cachedCommands.insert(node->cachedCommands.end(), cmds.begin(), cmds.end());
                        }
                        continue;
                    }

                    // If all are nullptr: node was deleted, skip it
                }

                node->isCommandsCacheValid = true;
                out.insert(out.end(), node->cachedCommands.begin(), node->cachedCommands.end());
            };

        collectFromContainer(rootRender, m_renderCommands);

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
    std::atomic<bool> m_running;

    NodeId m_rootId;
    NodeId m_rectId;

    std::unique_ptr<FrontendContainer> m_root;
    std::unique_ptr<FrontendShapeRect> m_rect;

    OpenGLRenderer m_renderer;
    std::vector<RenderCommand> m_renderCommands;
};

} // namespace ui
