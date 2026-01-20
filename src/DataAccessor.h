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
        : m_id(id)
        , m_renderContext(renderContext) {}

    DataT& AccessData() {
        const auto currentVersion = m_renderContext.Version();
        if (m_lastVersion == currentVersion) {
            assert(m_cached);
            return *m_cached;
        }

        m_cached = &m_renderContext.template AccessData<DataT>(m_id);
        m_lastVersion = currentVersion;

        return *m_cached;
    }

private:
    NodeId m_id;
    RenderContext& m_renderContext;
    DataT* m_cached = nullptr;
    std::size_t m_lastVersion = VERSION_UNDEFINED;
};

} // namespace ui
