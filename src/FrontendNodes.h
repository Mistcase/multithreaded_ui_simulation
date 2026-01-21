#pragma once

#include "TreeNode.h"
#include "ui_ids.h"

#include <memory>
#include <string>

// Forward declarations
namespace ui {
    class BackendContainerNode;
    class BackendTextNode;
    class BackendShapeNode;
    class BackendShapeRectNode;
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

    TreeNode* Backend() const { return m_backend.get(); }

protected:
    std::unique_ptr<TreeNode> m_backend;
};

class FrontendContainer : public FrontendNode {
public:
    // Factory method: creates backend and frontend, returns frontend
    static std::unique_ptr<FrontendContainer> Create(NodeId id);

    explicit FrontendContainer(std::unique_ptr<BackendContainerNode> backend);

    void AddChild(FrontendNode* child);

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

class FrontendShape : public FrontendNode {
public:
    // Factory method: creates backend and frontend, returns frontend
    static std::unique_ptr<FrontendShape> Create(NodeId id);

    explicit FrontendShape(std::unique_ptr<BackendShapeNode> backend);

private:
    BackendShapeNode* m_shapeBackend;  // Cached pointer to BackendShapeNode
};

class FrontendShapeRect : public FrontendShape {
public:
    // Factory method: creates backend and frontend, returns frontend
    static std::unique_ptr<FrontendShapeRect> Create(NodeId id);

    explicit FrontendShapeRect(std::unique_ptr<BackendShapeRectNode> backend);

    void SetWidth(float width);
    void SetHeight(float height);

private:
    BackendShapeRectNode* m_shapeRectBackend;  // Cached pointer to BackendShapeRectNode
};

} // namespace ui
