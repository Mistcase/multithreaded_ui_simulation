#include "FrontendNodes.h"
#include "ContainerNode.h"
#include "TextNode.h"

namespace ui {

std::unique_ptr<FrontendContainer> FrontendContainer::Create(NodeId id, RenderContext& ctx) {
    auto backend = std::make_unique<ContainerNode>(id, ctx);
    return std::make_unique<FrontendContainer>(std::move(backend));
}

FrontendContainer::FrontendContainer(std::unique_ptr<ContainerNode> backend)
    : FrontendNode(std::move(backend))
    , containerBackend_(static_cast<ContainerNode*>(FrontendNode::backend_.get())) {}

void FrontendContainer::AddChild(NodeId childId, bool isText) {
    if (containerBackend_) {
        containerBackend_->AddChild(childId, isText);
    }
}

std::unique_ptr<FrontendText> FrontendText::Create(NodeId id, RenderContext& ctx) {
    auto backend = std::make_unique<TextNode>(id, ctx);
    return std::make_unique<FrontendText>(std::move(backend));
}

FrontendText::FrontendText(std::unique_ptr<TextNode> backend)
    : FrontendNode(std::move(backend))
    , textBackend_(static_cast<TextNode*>(FrontendNode::backend_.get())) {}

void FrontendText::SetText(const std::string& text) {
    if (textBackend_) {
        textBackend_->SetText(text);
    }
}

} // namespace ui
