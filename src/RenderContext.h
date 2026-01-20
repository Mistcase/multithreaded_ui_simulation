#pragma once

#include "ui_ids.h"
#include "ChangeBuffer.h"
#include "NodeData.h"
#include "NodeIdAllocator.h"

#include <cstdint>
#include <functional>
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
struct RenderShapeNode;
struct RenderShapeRectNode;

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

struct RenderShapeNode {
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
};

struct RenderShapeRectNode {
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
    float width = 0.0f;
    float height = 0.0f;
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

template <>
struct RenderNodeTraits<ShapeNodeData> {
    using RenderNodeType = RenderShapeNode;
};

template <>
struct RenderNodeTraits<ShapeRectNodeData> {
    using RenderNodeType = RenderShapeRectNode;
};

// ---------------------------------
// TypeStorage: stores render nodes per type
// Similar to TypeBuffer but for render-side (read-only) data
// ---------------------------------

template <class RenderNodeType>
class TypeStorage {
public:
    RenderNodeType* EnsureRenderNode(NodeId id) {
        const std::uint64_t idx = ExtractIndex(id);
        const std::uint16_t gen = ExtractGeneration(id);

        // Expand vector if needed
        if (idx >= m_nodes.size()) {
            m_nodes.resize(idx + 1);
            m_generations.resize(idx + 1, 0);
        }

        // Check generation match
        if (m_generations[idx] != gen) {
            // Generation mismatch: reinitialize slot
            m_nodes[idx] = RenderNodeType{};
            m_generations[idx] = gen;
        }

        return &m_nodes[idx];
    }

    RenderNodeType* TryGetRenderNode(NodeId id) {
        const std::uint64_t idx = ExtractIndex(id);
        const std::uint16_t gen = ExtractGeneration(id);

        if (idx >= m_nodes.size() || m_generations[idx] != gen) {
            return nullptr;
        }

        return &m_nodes[idx];
    }

    void ClearNode(std::uint64_t idx, std::uint16_t newGeneration) {
        if (idx < m_generations.size()) {
            m_generations[idx] = newGeneration;
        }
        if (idx < m_nodes.size()) {
            m_nodes[idx] = RenderNodeType{};
        }
    }

    const std::vector<RenderNodeType>& GetNodes() const { return m_nodes; }

private:
    std::vector<RenderNodeType> m_nodes;
    std::vector<std::uint16_t> m_generations;  // generation per slot
};

// RenderContext: owns ChangeBuffer and render tree
// Singleton pattern - single instance for the entire application

class RenderContext {
public:
    // Get singleton instance
    static RenderContext& Instance() {
        static RenderContext instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    // Allocate new NodeId (delegates to allocator)
    NodeId AllocateNodeId() {
        return m_nodeIdAllocator.Allocate();
    }

    // Update thread API: access write-side data for any type
    // Automatically registers ProcessChanges<T> handler on first access
    template <typename T>
    T& AccessData(NodeId id) {
        RegisterTypeHandler<T>();
        return m_changeBuffer.AccessData<T>(id);
    }

    // Template methods for accessing render nodes by NodeData type
    template <typename T>
    auto EnsureRenderNode(NodeId id) -> typename RenderNodeTraits<T>::RenderNodeType* {
        return Storage<T>().EnsureRenderNode(id);
    }

    template <typename T>
    auto TryGetRenderNode(NodeId id) -> typename RenderNodeTraits<T>::RenderNodeType* {
        return Storage<T>().TryGetRenderNode(id);
    }

    // Update thread: called at the end of update
    // Under render mutex: applies changes to render tree.
    void Sync();

    // Render thread: read-only access to render tree under render mutex
    std::mutex& RenderMutex() { return m_renderMutex; }

private:
    // Private constructor for singleton
    RenderContext() = default;
    // Static method to get TypeStorage for a specific type
    template <typename T>
    static TypeStorage<typename RenderNodeTraits<T>::RenderNodeType>& Storage() {
        static TypeStorage<typename RenderNodeTraits<T>::RenderNodeType> storage;
        return storage;
    }

    // Template method to process changes for any type
    template <typename T>
    void ProcessChanges();

    // Register type handler for automatic processing in Sync()
    // Uses static variable to ensure registration happens only once per type
    template <typename T>
    void RegisterTypeHandler() {
        static bool registered = []() {
            // Register handler - will be added to instance's handler list
            // Note: Since this is called from instance method, Instance() is safe
            RenderContext& instance = Instance();
            instance.m_typeHandlers.push_back([](RenderContext* ctx) {
                ctx->ProcessChanges<T>();
            });
            return true;
        }();
        (void)registered; // Suppress unused variable warning
    }

    // Call all registered type handlers
    void ProcessAllRegisteredTypes() {
        for (auto& handler : m_typeHandlers) {
            handler(this);
        }
    }

    ChangeBuffer m_changeBuffer;
    NodeIdAllocator m_nodeIdAllocator;
    std::mutex m_renderMutex;
    std::vector<std::function<void(RenderContext*)>> m_typeHandlers;
};

} // namespace ui
