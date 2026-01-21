#pragma once

#include "ui_ids.h"
#include "NodeData.h"
#include "RenderContext.h"

#include <memory>

namespace ui {

// Backend node (base)
class TreeNode {
public:
    explicit TreeNode(NodeId id)
        : m_id(id) {}

    virtual ~TreeNode() = default;

    NodeId Id() const { return m_id; }
    void SetParent(TreeNode* parent) { m_parent = parent; }
    TreeNode* Parent() const { return m_parent; }

    virtual void SetPosition(float x, float y) = 0;
    virtual void SetVisible(bool v) = 0;
    virtual void Term() = 0;

protected:
    NodeId m_id;
    TreeNode* m_parent = nullptr;

    void InvalidateCacheUpwards() {
        TreeNode* current = m_parent;
        while (current) {
            auto& data = RenderContext::Instance().AccessData<ContainerNodeData>(current->Id());
            data.invalidateCommandsCache = true;
            current = current->m_parent;
        }
    }
};

} // namespace ui
