#pragma once

#include "ui_ids.h"

#include <cstdint>
#include <vector>

namespace ui {

// Allocates NodeId handles with generation tracking
// Manages free list to reuse indices and prevent holes in vectors
class NodeIdAllocator {
public:
    NodeIdAllocator() = default;

    // Allocate a new NodeId
    // Returns handle with unique index and generation=0
    NodeId Allocate() {
        std::uint64_t index;
        
        if (!m_freeIndices.empty()) {
            // Reuse freed index
            index = m_freeIndices.back();
            m_freeIndices.pop_back();
        } else {
            // Allocate new index
            index = m_nextIndex++;
        }
        
        // Ensure vectors are large enough
        if (index >= m_generations.size()) {
            m_generations.resize(index + 1, 0);
        }
        
        // Return handle with current generation (should be 0 for new slots)
        return MakeNodeId(index, m_generations[index]);
    }

    // Free a NodeId (called when node is deleted)
    // Increments generation to invalidate all existing handles with this index
    void Free(NodeId id) {
        const std::uint64_t idx = ExtractIndex(id);
        const std::uint16_t gen = ExtractGeneration(id);
        
        if (idx >= m_generations.size()) {
            return;  // Invalid index
        }
        
        // Check generation match (safety check)
        if (m_generations[idx] != gen) {
            return;  // Already freed or invalid
        }
        
        // Increment generation to invalidate all existing handles
        // Note: & 0xFFFF is redundant for uint16_t (overflow is well-defined wrap-around),
        // but kept for explicitness and type safety if generation type changes
        m_generations[idx] = (gen + 1) & 0xFFFF;
        
        // Add to free list for reuse
        m_freeIndices.push_back(idx);
    }

    // Get current generation for an index (for validation)
    std::uint16_t GetGeneration(std::uint64_t index) const {
        if (index >= m_generations.size()) {
            return 0;
        }
        return m_generations[index];
    }

private:
    std::uint64_t m_nextIndex = 0;  // Next index to allocate
    std::vector<std::uint16_t> m_generations;  // Generation per index
    std::vector<std::uint64_t> m_freeIndices;  // Free list for reuse
};

} // namespace ui
