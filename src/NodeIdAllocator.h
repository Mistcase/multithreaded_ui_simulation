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
        
        if (!freeIndices_.empty()) {
            // Reuse freed index
            index = freeIndices_.back();
            freeIndices_.pop_back();
        } else {
            // Allocate new index
            index = nextIndex_++;
        }
        
        // Ensure vectors are large enough
        if (index >= generations_.size()) {
            generations_.resize(index + 1, 0);
        }
        
        // Return handle with current generation (should be 0 for new slots)
        return MakeNodeId(index, generations_[index]);
    }

    // Free a NodeId (called when node is deleted)
    // Increments generation to invalidate all existing handles with this index
    void Free(NodeId id) {
        const std::uint64_t idx = ExtractIndex(id);
        const std::uint16_t gen = ExtractGeneration(id);
        
        if (idx >= generations_.size()) {
            return;  // Invalid index
        }
        
        // Check generation match (safety check)
        if (generations_[idx] != gen) {
            return;  // Already freed or invalid
        }
        
        // Increment generation to invalidate all existing handles
        // Note: & 0xFFFF is redundant for uint16_t (overflow is well-defined wrap-around),
        // but kept for explicitness and type safety if generation type changes
        generations_[idx] = (gen + 1) & 0xFFFF;
        
        // Add to free list for reuse
        freeIndices_.push_back(idx);
    }

    // Get current generation for an index (for validation)
    std::uint16_t GetGeneration(std::uint64_t index) const {
        if (index >= generations_.size()) {
            return 0;
        }
        return generations_[index];
    }

private:
    std::uint64_t nextIndex_ = 0;  // Next index to allocate
    std::vector<std::uint16_t> generations_;  // Generation per index
    std::vector<std::uint64_t> freeIndices_;  // Free list for reuse
};

} // namespace ui
