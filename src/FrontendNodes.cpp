#include "FrontendNodes.h"
#include "BackendContainerNode.h"
#include "BackendTextNode.h"
#include "BackendShapeNode.h"
#include "BackendShapeRectNode.h"

namespace ui {

std::unique_ptr<FrontendContainer> FrontendContainer::Create(NodeId id) {
    auto backend = std::make_unique<BackendContainerNode>(id);
    return std::make_unique<FrontendContainer>(std::move(backend));
}

FrontendContainer::FrontendContainer(std::unique_ptr<BackendContainerNode> backend)
    : FrontendNode(std::move(backend))
    , m_containerBackend(static_cast<BackendContainerNode*>(FrontendNode::m_backend.get())) {}

void FrontendContainer::AddChild(FrontendNode* child) {
    if (m_containerBackend && child->Backend()) {
        m_containerBackend->AddChild(child->Backend());
    }
}

std::unique_ptr<FrontendText> FrontendText::Create(NodeId id) {
    auto backend = std::make_unique<BackendTextNode>(id);
    return std::make_unique<FrontendText>(std::move(backend));
}

FrontendText::FrontendText(std::unique_ptr<BackendTextNode> backend)
    : FrontendNode(std::move(backend))
    , m_textBackend(static_cast<BackendTextNode*>(FrontendNode::m_backend.get())) {}

void FrontendText::SetText(const std::string& text) {
    if (m_textBackend) {
        m_textBackend->SetText(text);
    }
}

std::unique_ptr<FrontendShape> FrontendShape::Create(NodeId id) {
    auto backend = std::make_unique<BackendShapeNode>(id);
    return std::make_unique<FrontendShape>(std::move(backend));
}

FrontendShape::FrontendShape(std::unique_ptr<BackendShapeNode> backend)
    : FrontendNode(std::move(backend))
    , m_shapeBackend(static_cast<BackendShapeNode*>(FrontendNode::m_backend.get())) {}

std::unique_ptr<FrontendShapeRect> FrontendShapeRect::Create(NodeId id) {
    auto backend = std::make_unique<BackendShapeRectNode>(id);
    return std::make_unique<FrontendShapeRect>(std::move(backend));
}

FrontendShapeRect::FrontendShapeRect(std::unique_ptr<BackendShapeRectNode> backend)
    : FrontendShape(std::unique_ptr<BackendShapeNode>(backend.release()))
    , m_shapeRectBackend(static_cast<BackendShapeRectNode*>(FrontendNode::m_backend.get())) {}

void FrontendShapeRect::SetWidth(float width) {
    if (m_shapeRectBackend) {
        m_shapeRectBackend->SetWidth(width);
    }
}

void FrontendShapeRect::SetHeight(float height) {
    if (m_shapeRectBackend) {
        m_shapeRectBackend->SetHeight(height);
    }
}

} // namespace ui
