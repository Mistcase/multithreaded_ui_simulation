#pragma once

#include "TreeNode.h"
#include "RenderContext.h"
#include "NodeData.h"
#include "BackendTextNode.h" // for RTTI type check

#include <memory>
#include <vector>

namespace ui {

class BackendContainerNode : public TreeNode {
public:
    explicit BackendContainerNode(NodeId id)
        : TreeNode(id) {}

    void SetPosition(float x, float y) override {
        auto& data = RenderContext::Instance().AccessData<ContainerNodeData>(m_id);
        data.x = x;
        data.y = y;
    }

    void SetVisible(bool v) override {
        auto& data = RenderContext::Instance().AccessData<ContainerNodeData>(m_id);
        data.visible = v;
    }

    void AddChild(NodeId childId) {
        auto& data = RenderContext::Instance().AccessData<ContainerNodeData>(m_id);
        data.children.push_back(childId);
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = RenderContext::Instance().AccessData<ContainerNodeData>(m_id);
        data.deleted = true;
    }
};

} // namespace ui

