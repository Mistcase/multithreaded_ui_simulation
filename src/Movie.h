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
        : running_(true)
        , rootId_(renderContext_.AllocateNodeId())  // Allocated by RenderContext
        , textId_(renderContext_.AllocateNodeId()) {  // Allocated by RenderContext
        // Frontend nodes create and own backend nodes
        root_ = FrontendContainer::Create(rootId_, renderContext_);
        text_ = FrontendText::Create(textId_, renderContext_);

        root_->AddChild(textId_);

        text_->SetText("Hello UI");
        root_->SetPosition(0.0f, 0.0f);
        text_->SetPosition(10.0f, 20.0f);
    }

    void Stop() { running_ = false; }
    bool IsRunning() const { return running_.load(); }

    // main_thread: update
    void Update() {
        TRACE_SCOPE("Movie::Update");
        SimulateScriptLanguageProcessing();

        // Fake animation: move text along X axis
        static float x = 10.0f;
        static int frameCount = 0;
        frameCount++;
        x += 1.0f;
        text_->SetPosition(x, 20.0f);
        
        // Example: delete text node after 3 frames
        if (frameCount == 3 && text_) {
            text_->Term();
            text_.reset();  // Clear frontend pointer
        }

        // At the end of update: sync buffer -> render tree
        renderContext_.Sync();
    }

    // render_thread: render
    void Render() {
        TRACE_SCOPE("Movie::Render");
        {
            std::lock_guard<std::mutex> lock(renderContext_.RenderMutex());
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
        auto* rootRender = renderContext_.TryGetNode<ContainerNodeData>(rootId_);
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
                const RenderContainerNode* container = renderContext_.TryGetNode<ContainerNodeData>(childId);
                if (container) {
                    stack.push_back(container);
                    continue;
                }
                
                // Try text node
                const RenderTextNode* text = renderContext_.TryGetNode<TextNodeData>(childId);
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
    std::atomic<bool> running_;

    RenderContext renderContext_;
    NodeId rootId_;
    NodeId textId_;

    std::unique_ptr<FrontendContainer> root_;
    std::unique_ptr<FrontendText> text_;
};

} // namespace ui
