#pragma once

#include "TreeNode.h"
#include "RenderContext.h"
#include "NodeData.h"

#include <memory>
#include <string>

namespace ui {

class BackendTextNode : public TreeNode {
public:
    explicit BackendTextNode(NodeId id)
        : TreeNode(id) {}

    void SetPosition(float x, float y) override {
        auto& data = RenderContext::Instance().AccessData<TextNodeData>(m_id);
        data.x = x;
        data.y = y;
        data.invalidateCommandsCache = true;
        InvalidateCacheUpwards();
    }

    void SetVisible(bool v) override {
        auto& data = RenderContext::Instance().AccessData<TextNodeData>(m_id);
        data.visible = v;
        data.invalidateCommandsCache = true;
        InvalidateCacheUpwards();
    }

    void SetText(const std::string& text) {
        auto& data = RenderContext::Instance().AccessData<TextNodeData>(m_id);
        data.text = text;
        data.invalidateCommandsCache = true;
        InvalidateCacheUpwards();
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = RenderContext::Instance().AccessData<TextNodeData>(m_id);
        data.deleted = true;
        data.invalidateCommandsCache = true;
        InvalidateCacheUpwards();
    }
};

} // namespace ui
