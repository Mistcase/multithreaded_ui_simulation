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
        , data_(id, ctx) {}

    void SetPosition(float x, float y) override {
        auto& data = data_.AccessData();
        data.x = x;
        data.y = y;
    }

    void SetVisible(bool v) override {
        auto& data = data_.AccessData();
        data.visible = v;
    }

    void AddChild(NodeId childId) {
        auto& data = data_.AccessData();
        data.children.push_back(ChildLink{childId});
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = data_.AccessData();
        data.deleted = true;
    }

private:
    ContainerDataAccessor data_;
};

} // namespace ui

