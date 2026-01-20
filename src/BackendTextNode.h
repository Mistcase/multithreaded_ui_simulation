#pragma once

#include "TreeNode.h"
#include "RenderContext.h"
#include "ChangeBuffer.h"

#include <memory>
#include <string>

namespace ui {

class BackendTextNode : public TreeNode {
public:
    BackendTextNode(NodeId id, RenderContext& ctx)
        : TreeNode(id, ctx) {}

    void SetPosition(float x, float y) override {
        auto& data = m_renderContext.AccessData<TextNodeData>(m_id);
        data.x = x;
        data.y = y;
    }

    void SetVisible(bool v) override {
        auto& data = m_renderContext.AccessData<TextNodeData>(m_id);
        data.visible = v;
    }

    void SetText(const std::string& text) {
        auto& data = m_renderContext.AccessData<TextNodeData>(m_id);
        data.text = text;
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = m_renderContext.AccessData<TextNodeData>(m_id);
        data.deleted = true;
    }
};

} // namespace ui
