#pragma once

#include "ui_ids.h"

#include <memory>

namespace ui {

class RenderContext;

// Backend node (base)
class TreeNode {
public:
    TreeNode(NodeId id, RenderContext& renderContext)
        : id_(id)
        , renderContext_(renderContext) {}

    virtual ~TreeNode() = default;

    NodeId Id() const { return id_; }

    virtual void SetPosition(float x, float y) = 0;
    virtual void SetVisible(bool v) = 0;
    virtual void Term() = 0;

protected:
    NodeId id_;
    RenderContext& renderContext_;
};

} // namespace ui
