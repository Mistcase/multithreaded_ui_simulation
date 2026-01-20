#include "RenderContext.h"
#include "NodeData.h"

#include "TraceProfiler.h"

#include <chrono>
#include <thread>

namespace ui {

void RenderContext::Sync() {
	std::lock_guard lock(m_renderMutex);

    TRACE_SCOPE("RenderContext::Sync");

	// Process all types that were accessed via AccessData<T>
	// Handlers are automatically registered on first AccessData<T> call
	ProcessAllRegisteredTypes();

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
}

} // namespace ui
