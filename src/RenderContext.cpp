#include "RenderContext.h"
#include "NodeData.h"

#include "TraceProfiler.h"

#include <chrono>
#include <thread>

namespace ui {

// Template method implementation for processing changes
template <typename T>
void RenderContext::ProcessChanges() {
	auto changes = m_changeBuffer.Snapshot<T>();

	for (auto& change : changes) {
		if (change.deleted) {
			const std::uint64_t idx = ExtractIndex(change.id);
			m_nodeIdAllocator.Free(change.id);

			// Remove render node
			Storage<T>().ClearNode(idx, m_nodeIdAllocator.GetGeneration(idx));
			continue;
		}


		change.Flush(*this);
	}
}

void RenderContext::Sync() {
	std::lock_guard lock(m_renderMutex);

    TRACE_SCOPE("RenderContext::Sync");

	// Process all types using template method
	ProcessChanges<ContainerNodeData>();
	ProcessChanges<TextNodeData>();
	ProcessChanges<ShapeNodeData>();
	ProcessChanges<ShapeRectNodeData>();

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
}

} // namespace ui
