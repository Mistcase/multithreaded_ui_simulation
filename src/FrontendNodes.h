#pragma once

#include "TreeNode.h"
#include "ui_ids.h"

#include <memory>
#include <string>

// Forward declarations
namespace ui {
    class ContainerNode;
    class TextNode;
}

namespace ui {

// Frontend wrapper visible to gameplay developers
class FrontendNode {
public:
    explicit FrontendNode(std::unique_ptr<TreeNode> backend)
        : backend_(std::move(backend)) {}

    virtual ~FrontendNode() = default;

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
    
    explicit FrontendContainer(std::unique_ptr<ContainerNode> backend);
    
    void AddChild(NodeId childId, bool isText);

private:
    ContainerNode* containerBackend_;  // Cached pointer to ContainerNode
};

class FrontendText : public FrontendNode {
public:
    // Factory method: creates backend and frontend, returns frontend
    static std::unique_ptr<FrontendText> Create(NodeId id, RenderContext& ctx);
    
    explicit FrontendText(std::unique_ptr<TextNode> backend);
    
    void SetText(const std::string& text);

private:
    TextNode* textBackend_;  // Cached pointer to TextNode
};

} // namespace ui
