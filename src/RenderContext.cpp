#include "RenderContext.h"

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

			// Update generation in storage
			Storage<T>().ClearNode(idx, m_nodeIdAllocator.GetGeneration(idx));
		} else {
			change.Flush(*this);
		}
	}
}

void RenderContext::Sync() {
	std::lock_guard lock(m_renderMutex);

    TRACE_SCOPE("RenderContext::Sync");

	// Process all types using template method
	ProcessChanges<ContainerNodeData>();
	ProcessChanges<TextNodeData>();

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
}

void ContainerNodeData::Flush(RenderContext& ctx) {
    RenderContainerNode* r = render ? render : ctx.EnsureRenderNode<ContainerNodeData>(id);
    render = r;
    r->x = x;
    r->y = y;
    r->visible = visible;

    // Resize to target size, reusing existing capacity when possible
    r->children.resize(children.size());

    // Update children in place - store only NodeId
    for (std::size_t i = 0; i < children.size(); ++i) {
        r->children[i] = children[i].id;
    }
}

void TextNodeData::Flush(RenderContext& ctx) {
    RenderTextNode* r = render ? render : ctx.EnsureRenderNode<TextNodeData>(id);
    render = r;
    r->x = x;
    r->y = y;
    r->visible = visible;
    r->text = text;
}

} // namespace ui
