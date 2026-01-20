#pragma once

#include "ui_ids.h"

#include <memory>

namespace ui {

class RenderContext;

// Backend node (base)
class TreeNode {
public:
    TreeNode(NodeId id, RenderContext& renderContext)
        : m_id(id)
        , m_renderContext(renderContext) {}

    virtual ~TreeNode() = default;

    NodeId Id() const { return m_id; }

    virtual void SetPosition(float x, float y) = 0;
    virtual void SetVisible(bool v) = 0;
    virtual void Term() = 0;

protected:
    NodeId m_id;
    RenderContext& m_renderContext;
};

} // namespace ui
