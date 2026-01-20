#pragma once

#include "TreeNode.h"
#include "RenderContext.h"
#include "ChangeBuffer.h"
#include "DataAccessor.h"
#include "TextNode.h" // for RTTI type check

#include <memory>
#include <vector>

namespace ui {

using ContainerDataAccessor = DataAccessor<RenderContext, ContainerNodeData, &RenderContext::AccessContainerData>;

class ContainerNode : public TreeNode {
public:
    ContainerNode(NodeId id, RenderContext& ctx)
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

    void AddChild(NodeId childId, bool isText) {
        auto& data = data_.AccessData();
        data.children.push_back(ChildLink{childId, isText});
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

