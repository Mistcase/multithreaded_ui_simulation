#pragma once

#include "TreeNode.h"
#include "TextNode.h"

#include <memory>

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
    explicit FrontendContainer(std::unique_ptr<ContainerNode> backend)
        : FrontendNode(std::move(backend)) {}
};

class FrontendText : public FrontendNode {
public:
    explicit FrontendText(std::unique_ptr<TextNode> backend)
        : FrontendNode(std::move(backend))
        , textBackend_(static_cast<TextNode*>(FrontendNode::backend_.get())) {}

    void SetText(const std::string& text) {
        if (textBackend_) {
            textBackend_->SetText(text);
        }
    }

private:
    TextNode* textBackend_;  // Cached pointer to TextNode
};

} // namespace ui
