#pragma once

#include "ui_ids.h"

#include <cassert>
#include <cstddef>

namespace ui {

class RenderContext;

// Generic accessor for write-side NodeData.
// Caches a pointer to data and invalidates it by ChangeBuffer/RenderContext version.
template <class DataT>
class DataAccessor {
public:
    static constexpr std::size_t VERSION_UNDEFINED = static_cast<std::size_t>(-1);

    DataAccessor(NodeId id, RenderContext& renderContext)
        : id_(id)
        , renderContext_(renderContext) {}

    DataT& AccessData() {
        const auto currentVersion = renderContext_.Version();
        if (lastVersion_ == currentVersion) {
            assert(cached_);
            return *cached_;
        }

        cached_ = &renderContext_.template AccessData<DataT>(id_);
        lastVersion_ = currentVersion;

        return *cached_;
    }

private:
    NodeId id_;
    RenderContext& renderContext_;
    DataT* cached_ = nullptr;
    std::size_t lastVersion_ = VERSION_UNDEFINED;
};

} // namespace ui
