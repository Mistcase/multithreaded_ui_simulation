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
            items_[index] = T{}; // TODO: Just to default value?
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
    // Template method to access data for any type
    template <typename T>
    T& AccessData(NodeId id);

    // Template method to snapshot and clear data for any type
    template <typename T>
    std::vector<T> Snapshot();

    bool Empty() const;

    std::size_t Version() const { return version_; }

private:
    // Static method to get TypeBuffer for a specific type
    template <typename T>
    static TypeBuffer<T>& Buffer() {
        static TypeBuffer<T> buffer;
        return buffer;
    }

    std::size_t version_ = 0; // Need it for fast DataAccessor validation
};

// Template method implementations
template <typename T>
inline T& ChangeBuffer::AccessData(NodeId id) {
    return Buffer<T>().AccessData(id);
}

template <typename T>
inline std::vector<T> ChangeBuffer::Snapshot() {
    ++version_;
    return Buffer<T>().SnapshotAndClear();
}

inline bool ChangeBuffer::Empty() const {
    // Check known types - this is a limitation of static approach
    // but avoids type erasure
    return Buffer<ContainerNodeData>().Empty() && Buffer<TextNodeData>().Empty();
}

} // namespace ui
