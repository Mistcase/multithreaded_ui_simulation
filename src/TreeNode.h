#pragma once

#include "ui_ids.h"

namespace ui {

// Backend node (base)
class TreeNode {
public:
    explicit TreeNode(NodeId id)
        : m_id(id) {}

    virtual ~TreeNode() = default;

    NodeId Id() const { return m_id; }

    virtual void SetPosition(float x, float y) = 0;
    virtual void SetVisible(bool v) = 0;
    virtual void Term() = 0;

protected:
    NodeId m_id;
};

} // namespace ui
