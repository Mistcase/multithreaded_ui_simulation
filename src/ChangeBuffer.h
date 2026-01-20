#pragma once

#include "ui_ids.h"

#include <cstddef>
#include <string>
#include <vector>

namespace ui {

class RenderContext;
struct RenderContainerNode;
struct RenderTextNode;

// ----------------------------
// Update-side (write) NodeData
// No virtuals and no base class hierarchy
// ----------------------------

struct ChildLink {
    NodeId id{};
    bool isText = false;
};

struct ContainerNodeData {
    NodeId id{};
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
    bool deleted = false;  // Mark for deletion
    std::vector<ChildLink> children;
    RenderContainerNode* render = nullptr;

    void Flush(RenderContext& ctx);
};

struct TextNodeData {
    NodeId id{};
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
    bool deleted = false;  // Mark for deletion
    std::string text;
    RenderTextNode* render = nullptr;

    void Flush(RenderContext& ctx);
};

template <class T>
class TypeBuffer {
public:
    T& AccessData(NodeId id) {
        const std::size_t index = static_cast<std::size_t>(ExtractIndex(id));

        // Ensure storage is large enough for this handle index.
        if (index >= items_.size()) {
            items_.resize(index + 1);
            dirty_.resize(index + 1, false);
        }

        // First access for this handle in the current frame: register in active list
        // and reset per-frame data to a clean state.
        if (!dirty_[index]) {
            dirty_[index] = true;
            activeIndices_.push_back(index);

            // Reinitialize per-frame data for this node.
            items_[index] = T{};
            items_[index].id = id;
        }

        return items_[index];
    }

    std::vector<T> SnapshotAndClear() {
        std::vector<T> out;
        out.reserve(activeIndices_.size());

        // Collect only nodes that were actually touched in this frame.
        for (std::size_t index : activeIndices_) {
            out.push_back(std::move(items_[index]));

            // Reset slot state for the next frame.
            items_[index] = T{};
            dirty_[index] = false;
        }

        activeIndices_.clear();
        return out;
    }

    bool Empty() const { return activeIndices_.empty(); }

private:
    std::vector<T> items_;
    // Marks which indices were modified in the current frame.
    std::vector<bool> dirty_;
    // Compact list of indices that were touched in the current frame.
    std::vector<std::size_t> activeIndices_;
};

// ---------------------------------
// ChangeBuffer: stores batched changes per type
// ---------------------------------

class ChangeBuffer {
public:
    ContainerNodeData& AccessContainerData(NodeId id) {
        return containers_.AccessData(id);
    }

    TextNodeData& AccessTextData(NodeId id) {
        return texts_.AccessData(id);
    }

    std::vector<ContainerNodeData> SnapshotContainers() {
        ++version_;
        return containers_.SnapshotAndClear();
    }

    std::vector<TextNodeData> SnapshotTexts() {
        ++version_;
        return texts_.SnapshotAndClear();
    }

    bool Empty() const {
        return containers_.Empty() && texts_.Empty();
    }

    std::size_t Version() const { return version_; }

private:
    TypeBuffer<ContainerNodeData> containers_;
    TypeBuffer<TextNodeData> texts_;
    std::size_t version_ = 0;
};

} // namespace ui
