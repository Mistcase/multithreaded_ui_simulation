#pragma once

#include "TreeNode.h"
#include "RenderContext.h"
#include "ChangeBuffer.h"
#include "DataAccessor.h"
#include "BackendTextNode.h" // for RTTI type check

#include <memory>
#include <vector>

namespace ui {

using ContainerDataAccessor = DataAccessor<ContainerNodeData>;

class BackendContainerNode : public TreeNode {
public:
    BackendContainerNode(NodeId id, RenderContext& ctx)
        : TreeNode(id, ctx)
        , m_data(id, ctx) {}

    void SetPosition(float x, float y) override {
        auto& data = m_data.AccessData();
        data.x = x;
        data.y = y;
    }

    void SetVisible(bool v) override {
        auto& data = m_data.AccessData();
        data.visible = v;
    }

    void AddChild(NodeId childId) {
        auto& data = m_data.AccessData();
        data.children.push_back(ChildLink{childId});
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = m_data.AccessData();
        data.deleted = true;
    }

private:
    ContainerDataAccessor m_data;
};

} // namespace ui

