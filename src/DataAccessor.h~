#pragma once

#include "ui_ids.h"

#include <cassert>
#include <cstddef>

namespace ui {

// Generic accessor for write-side NodeData.
// Caches a pointer to data and invalidates it by ChangeBuffer/RenderContext version.
template <class BufferT, class DataT, DataT& (BufferT::*AccessFn)(NodeId)>
class DataAccessor {
public:
    static constexpr std::size_t VERSION_UNDEFINED = static_cast<std::size_t>(-1);

    DataAccessor(NodeId id, BufferT& buffer)
        : id_(id)
        , buffer_(buffer) {}

    DataT& AccessData() {
        const auto currentVersion = buffer_.Version();
        if (lastVersion_ == currentVersion) {
            assert(cached_);
            return *cached_;
        }

        cached_ = &(buffer_.*AccessFn)(id_);
        lastVersion_ = currentVersion;

        return *cached_;
    }

private:
    NodeId id_;
    BufferT& buffer_;
    DataT* cached_ = nullptr;
    std::size_t lastVersion_ = VERSION_UNDEFINED;
};

} // namespace ui
