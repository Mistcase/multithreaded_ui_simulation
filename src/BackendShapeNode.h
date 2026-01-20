#pragma once

#include "TreeNode.h"
#include "RenderContext.h"
#include "NodeData.h"

#include <memory>

namespace ui {

class BackendShapeNode : public TreeNode {
public:
    explicit BackendShapeNode(NodeId id)
        : TreeNode(id) {}

    void SetPosition(float x, float y) override {
        auto& data = RenderContext::Instance().AccessData<ShapeNodeData>(m_id);
        data.x = x;
        data.y = y;
    }

    void SetVisible(bool v) override {
        auto& data = RenderContext::Instance().AccessData<ShapeNodeData>(m_id);
        data.visible = v;
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = RenderContext::Instance().AccessData<ShapeNodeData>(m_id);
        data.deleted = true;
    }
};

} // namespace ui
