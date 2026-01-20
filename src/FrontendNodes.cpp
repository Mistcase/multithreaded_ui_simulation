#include "FrontendNodes.h"
#include "BackendContainerNode.h"
#include "BackendTextNode.h"

namespace ui {

std::unique_ptr<FrontendContainer> FrontendContainer::Create(NodeId id, RenderContext& ctx) {
    auto backend = std::make_unique<BackendContainerNode>(id, ctx);
    return std::make_unique<FrontendContainer>(std::move(backend));
}

FrontendContainer::FrontendContainer(std::unique_ptr<BackendContainerNode> backend)
    : FrontendNode(std::move(backend))
    , containerBackend_(static_cast<BackendContainerNode*>(FrontendNode::backend_.get())) {}

void FrontendContainer::AddChild(NodeId childId) {
    if (containerBackend_) {
        containerBackend_->AddChild(childId);
    }
}

std::unique_ptr<FrontendText> FrontendText::Create(NodeId id, RenderContext& ctx) {
    auto backend = std::make_unique<BackendTextNode>(id, ctx);
    return std::make_unique<FrontendText>(std::move(backend));
}

FrontendText::FrontendText(std::unique_ptr<BackendTextNode> backend)
    : FrontendNode(std::move(backend))
    , textBackend_(static_cast<BackendTextNode*>(FrontendNode::backend_.get())) {}

void FrontendText::SetText(const std::string& text) {
    if (textBackend_) {
        textBackend_->SetText(text);
    }
}

} // namespace ui
