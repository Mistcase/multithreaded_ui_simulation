#pragma once

#include <cstdint>

namespace ui {

// Handle-based ID: (index << 16) | generation
// Allows O(1) direct vector access without hash maps
using NodeId = std::uint64_t;

inline constexpr std::uint64_t ExtractIndex(NodeId id) {
    return id >> 16;
}

inline constexpr std::uint16_t ExtractGeneration(NodeId id) {
    return static_cast<std::uint16_t>(id & 0xFFFF);
}

inline constexpr NodeId MakeNodeId(std::uint64_t index, std::uint16_t generation) {
    return (index << 16) | generation;
}

} // namespace ui

