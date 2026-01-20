#pragma once

#include "TreeNode.h"
#include "ui_ids.h"

#include <memory>
#include <string>

// Forward declarations
namespace ui {
    class BackendContainerNode;
    class BackendTextNode;
}

namespace ui {

// Frontend wrapper visible to gameplay developers
class FrontendNode {
public:
    explicit FrontendNode(std::unique_ptr<TreeNode> backend)
        : backend_(std::move(backend)) {}

    virtual ~FrontendNode() {
        // Ensure backend node is properly terminated and freed even
        // if the owner forgets to call Term() explicitly.
        Term();
    }

    void SetPosition(float x, float y) {
        if (backend_) {
            backend_->SetPosition(x, y);
        }
    }

    void SetVisible(bool v) {
        if (backend_) {
            backend_->SetVisible(v);
        }
    }

    void Term() {
        if (backend_) {
            backend_->Term();
            backend_.reset();  // Delete backend node
        }
    }

protected:
    std::unique_ptr<TreeNode> backend_;
};

class FrontendContainer : public FrontendNode {
public:
    // Factory method: creates backend and frontend, returns frontend
    static std::unique_ptr<FrontendContainer> Create(NodeId id, RenderContext& ctx);
    
    explicit FrontendContainer(std::unique_ptr<BackendContainerNode> backend);
    
    void AddChild(NodeId childId);

private:
    BackendContainerNode* containerBackend_;  // Cached pointer to BackendContainerNode
};

class FrontendText : public FrontendNode {
public:
    // Factory method: creates backend and frontend, returns frontend
    static std::unique_ptr<FrontendText> Create(NodeId id, RenderContext& ctx);
    
    explicit FrontendText(std::unique_ptr<BackendTextNode> backend);
    
    void SetText(const std::string& text);

private:
    BackendTextNode* textBackend_;  // Cached pointer to BackendTextNode
};

} // namespace ui
