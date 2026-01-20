#pragma once

#include "ui_ids.h"
#include "ChangeBuffer.h"
#include "NodeIdAllocator.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ui {

// ----------------------------
// Render-side (read-only) data
// Types do NOT have to match write-side; we keep one type per node kind
// ----------------------------

struct RenderContainerNode;
struct RenderTextNode;

struct RenderContainerNode {
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
    std::vector<NodeId> children;  // Store only NodeId, resolve type dynamically
};

struct RenderTextNode {
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
    std::string text;
};

// Traits for mapping NodeData types to RenderNode types and storage
template <typename T>
struct RenderNodeTraits;

template <>
struct RenderNodeTraits<ContainerNodeData> {
    using RenderNodeType = RenderContainerNode;
};

template <>
struct RenderNodeTraits<TextNodeData> {
    using RenderNodeType = RenderTextNode;
};

// RenderContext: owns ChangeBuffer and render tree

class RenderContext {
public:
    RenderContext() = default;

    // Allocate new NodeId (delegates to allocator)
    NodeId AllocateNodeId() {
        return m_nodeIdAllocator.Allocate();
    }

    // Update thread API: access write-side data for any type
    template <typename T>
    T& AccessData(NodeId id) {
        return m_changeBuffer.AccessData<T>(id);
    }

    // Template methods for accessing render nodes by NodeData type
    template <typename T>
    auto EnsureNode(NodeId id) -> typename RenderNodeTraits<T>::RenderNodeType*;

    template <typename T>
    auto TryGetNode(NodeId id) -> typename RenderNodeTraits<T>::RenderNodeType*;

    // Update thread: called at the end of update
    // Under render mutex: applies changes to render tree.
    void Sync();

    std::size_t Version() const { return m_changeBuffer.Version(); }

    // Render thread: read-only access to render tree under render mutex
    std::mutex& RenderMutex() { return m_renderMutex; }
    const std::vector<RenderContainerNode>& RenderContainers() const { return m_renderContainers; }
    const std::vector<RenderTextNode>& RenderTexts() const { return m_renderTexts; }

private:
    // Template method to process changes for any type
    template <typename T>
    void ProcessChanges();

    ChangeBuffer m_changeBuffer;
    NodeIdAllocator m_nodeIdAllocator;
    std::mutex m_renderMutex;

    // Handle-based storage: direct vector access by index, generation check for validation
    // Generations are synced with NodeIdAllocator::m_generations
    std::vector<RenderContainerNode> m_renderContainers;
    std::vector<std::uint16_t> m_containerGenerations;  // generation per slot

    std::vector<RenderTextNode> m_renderTexts;
    std::vector<std::uint16_t> m_textGenerations;  // generation per slot
};

} // namespace ui
