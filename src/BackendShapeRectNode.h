#pragma once

#include "BackendShapeNode.h"
#include "RenderContext.h"
#include "NodeData.h"

#include <memory>

namespace ui {

class BackendShapeRectNode : public BackendShapeNode {
public:
    explicit BackendShapeRectNode(NodeId id)
        : BackendShapeNode(id) {}

    void SetPosition(float x, float y) override {
        auto& data = RenderContext::Instance().AccessData<ShapeRectNodeData>(m_id);
        data.x = x;
        data.y = y;
    }

    void SetVisible(bool v) override {
        auto& data = RenderContext::Instance().AccessData<ShapeRectNodeData>(m_id);
        data.visible = v;
    }

    void SetWidth(float width) {
        auto& data = RenderContext::Instance().AccessData<ShapeRectNodeData>(m_id);
        data.width = width;
    }

    void SetHeight(float height) {
        auto& data = RenderContext::Instance().AccessData<ShapeRectNodeData>(m_id);
        data.height = height;
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = RenderContext::Instance().AccessData<ShapeRectNodeData>(m_id);
        data.deleted = true;
    }
};

} // namespace ui
