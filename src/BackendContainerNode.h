#pragma once

#include "TreeNode.h"
#include "RenderContext.h"
#include "ChangeBuffer.h"
#include "BackendTextNode.h" // for RTTI type check

#include <memory>
#include <vector>

namespace ui {

class BackendContainerNode : public TreeNode {
public:
    BackendContainerNode(NodeId id, RenderContext& ctx)
        : TreeNode(id, ctx) {}

    void SetPosition(float x, float y) override {
        auto& data = m_renderContext.AccessData<ContainerNodeData>(m_id);
        data.x = x;
        data.y = y;
    }

    void SetVisible(bool v) override {
        auto& data = m_renderContext.AccessData<ContainerNodeData>(m_id);
        data.visible = v;
    }

    void AddChild(NodeId childId) {
        auto& data = m_renderContext.AccessData<ContainerNodeData>(m_id);
        data.children.push_back(ChildLink{childId});
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = m_renderContext.AccessData<ContainerNodeData>(m_id);
        data.deleted = true;
    }
};

} // namespace ui

