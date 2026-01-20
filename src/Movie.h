#pragma once

#include "RenderContext.h"
#include "BackendContainerNode.h"
#include "BackendTextNode.h"
#include "FrontendNodes.h"
#include "TraceProfiler.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

namespace ui {

// Simple scene / Movie
class Movie {
public:
    Movie()
        : m_running(true)
        , m_rootId(m_renderContext.AllocateNodeId())  // Allocated by RenderContext
        , m_textId(m_renderContext.AllocateNodeId()) {  // Allocated by RenderContext
        // Frontend nodes create and own backend nodes
        m_root = FrontendContainer::Create(m_rootId, m_renderContext);
        m_text = FrontendText::Create(m_textId, m_renderContext);

        m_root->AddChild(m_textId);

        m_text->SetText("Hello UI");
        m_root->SetPosition(0.0f, 0.0f);
        m_text->SetPosition(10.0f, 20.0f);
    }

    void Stop() { m_running = false; }
    bool IsRunning() const { return m_running.load(); }

    // main_thread: update
    void Update() {
        TRACE_SCOPE("Movie::Update");
        SimulateScriptLanguageProcessing();

        // Fake animation: move text along X axis
        static float x = 10.0f;
        static int frameCount = 0;
        frameCount++;
        x += 1.0f;
        m_text->SetPosition(x, 20.0f);

        // Example: delete text node after 3 frames
        if (frameCount == 3 && m_text) {
            m_text->Term();
            m_text.reset();  // Clear frontend pointer
        }

        // At the end of update: sync buffer -> render tree
        m_renderContext.Sync();
    }

    // render_thread: render
    void Render() {
        TRACE_SCOPE("Movie::Render");
        {
            std::lock_guard<std::mutex> lock(m_renderContext.RenderMutex());
            CollectRenderCommands();
        }

        // Execute render commands
        ExecuteRenderCommands();
    }

    void SimulateScriptLanguageProcessing() {
        TRACE_SCOPE("Movie::SimulateScriptLanguageProcessing");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

private:
    void CollectRenderCommands() {
        TRACE_SCOPE("Movie::CollectRenderCommands");
        auto* rootRender = m_renderContext.TryGetRenderNode<ContainerNodeData>(m_rootId);
        if (!rootRender) {
            return;
        }

        // DFS without map lookups: use already resolved child pointers
        std::vector<const RenderContainerNode*> stack;
        stack.push_back(rootRender);
        while (!stack.empty()) {
            const RenderContainerNode* node = stack.back();
            stack.pop_back();

            for (NodeId childId : node->children) {
                // Try container first (most common case for tree traversal)
                const RenderContainerNode* container = m_renderContext.TryGetRenderNode<ContainerNodeData>(childId);
                if (container) {
                    stack.push_back(container);
                    continue;
                }

                // Try text node
                const RenderTextNode* text = m_renderContext.TryGetRenderNode<TextNodeData>(childId);
                if (text) {
                    (void)text; // a real render command collection would go here
                }
                // If both are nullptr: node was deleted, skip it
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }

    void ExecuteRenderCommands() {
        TRACE_SCOPE("Movie::ExecuteRenderCommands");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

private:
    std::atomic<bool> m_running;

    RenderContext m_renderContext;
    NodeId m_rootId;
    NodeId m_textId;

    std::unique_ptr<FrontendContainer> m_root;
    std::unique_ptr<FrontendText> m_text;
};

} // namespace ui
