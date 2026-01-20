#pragma once

#include "TreeNode.h"
#include "RenderContext.h"
#include "ChangeBuffer.h"
#include "DataAccessor.h"

#include <memory>
#include <string>

namespace ui {

using TextDataAccessor = DataAccessor<TextNodeData>;

class BackendTextNode : public TreeNode {
public:
    BackendTextNode(NodeId id, RenderContext& ctx)
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

    void SetText(const std::string& text) {
        auto& data = data_.AccessData();
        data.text = text;
    }

    void Term() override {
        // Mark node as deleted in ChangeBuffer
        // On Sync, the render node will be removed
        auto& data = data_.AccessData();
        data.deleted = true;
    }

private:
    TextDataAccessor data_;
};

} // namespace ui
