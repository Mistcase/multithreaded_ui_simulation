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
        : m_backend(std::move(backend)) {}

    virtual ~FrontendNode() {
        // Ensure backend node is properly terminated and freed even
        // if the owner forgets to call Term() explicitly.
        Term();
    }

    void SetPosition(float x, float y) {
        if (m_backend) {
            m_backend->SetPosition(x, y);
        }
    }

    void SetVisible(bool v) {
        if (m_backend) {
            m_backend->SetVisible(v);
        }
    }

    void Term() {
        if (m_backend) {
            m_backend->Term();
            m_backend.reset();  // Delete backend node
        }
    }

protected:
    std::unique_ptr<TreeNode> m_backend;
};

class FrontendContainer : public FrontendNode {
public:
    // Factory method: creates backend and frontend, returns frontend
    static std::unique_ptr<FrontendContainer> Create(NodeId id);
    
    explicit FrontendContainer(std::unique_ptr<BackendContainerNode> backend);
    
    void AddChild(NodeId childId);

private:
    BackendContainerNode* m_containerBackend;  // Cached pointer to BackendContainerNode
};

class FrontendText : public FrontendNode {
public:
    // Factory method: creates backend and frontend, returns frontend
    static std::unique_ptr<FrontendText> Create(NodeId id);
    
    explicit FrontendText(std::unique_ptr<BackendTextNode> backend);
    
    void SetText(const std::string& text);

private:
    BackendTextNode* m_textBackend;  // Cached pointer to BackendTextNode
};

} // namespace ui
