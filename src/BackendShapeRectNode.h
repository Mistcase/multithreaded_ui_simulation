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
        data.invalidateCommandsCache = true;
        InvalidateCacheUpwards();
    }

    void SetVisible(bool v) override {
        auto& data = RenderContext::Instance().AccessData<ShapeRectNodeData>(m_id);
        data.visible = v;
        data.invalidateCommandsCache = true;
        InvalidateCacheUpwards();
    }

    void SetWidth(float width) {
        auto& data = RenderContext::Instance().AccessData<ShapeRectNodeData>(m_id);
        data.width = width;
        data.invalidateCommandsCache = true;
        InvalidateCacheUpwards();
    }

    void SetHeight(float height) {
        auto& data = RenderContext::Instance().AccessData<ShapeRectNodeData>(m_id);
        data.height = height;
        data.invalidateCommandsCache = true;
        InvalidateCacheUpwards();
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = RenderContext::Instance().AccessData<ShapeRectNodeData>(m_id);
        data.deleted = true;
        data.invalidateCommandsCache = true;
        InvalidateCacheUpwards();
    }
};

} // namespace ui
