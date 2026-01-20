#include "RenderContext.h"

#include "TraceProfiler.h"

#include <chrono>
#include <thread>

namespace ui {

// Template specialization for ContainerNodeData
template <>
RenderContainerNode* RenderContext::EnsureNode<ContainerNodeData>(NodeId id) {
    const std::uint64_t idx = ExtractIndex(id);
    const std::uint16_t gen = ExtractGeneration(id);

    // Expand vector if needed
    if (idx >= m_renderContainers.size()) {
        m_renderContainers.resize(idx + 1);
        m_containerGenerations.resize(idx + 1, 0);
    }

    // Check generation match
    if (m_containerGenerations[idx] != gen) {
        // Generation mismatch: reinitialize slot
        m_renderContainers[idx] = RenderContainerNode{};
        m_containerGenerations[idx] = gen;
    }

    return &m_renderContainers[idx];
}

// Template specialization for TextNodeData
template <>
RenderTextNode* RenderContext::EnsureNode<TextNodeData>(NodeId id) {
    const std::uint64_t idx = ExtractIndex(id);
    const std::uint16_t gen = ExtractGeneration(id);

    // Expand vector if needed
    if (idx >= m_renderTexts.size()) {
        m_renderTexts.resize(idx + 1);
        m_textGenerations.resize(idx + 1, 0);
    }

    // Check generation match
    if (m_textGenerations[idx] != gen) {
        // Generation mismatch: reinitialize slot
        m_renderTexts[idx] = RenderTextNode{};
        m_textGenerations[idx] = gen;
    }

    return &m_renderTexts[idx];
}

// Template specialization for ContainerNodeData
template <>
RenderContainerNode* RenderContext::TryGetNode<ContainerNodeData>(NodeId id) {
    const std::uint64_t idx = ExtractIndex(id);
    const std::uint16_t gen = ExtractGeneration(id);

    if (idx >= m_renderContainers.size() || m_containerGenerations[idx] != gen) {
        return nullptr;
    }

    return &m_renderContainers[idx];
}

// Template specialization for TextNodeData
template <>
RenderTextNode* RenderContext::TryGetNode<TextNodeData>(NodeId id) {
    const std::uint64_t idx = ExtractIndex(id);
    const std::uint16_t gen = ExtractGeneration(id);

    if (idx >= m_renderTexts.size() || m_textGenerations[idx] != gen) {
        return nullptr;
    }

    return &m_renderTexts[idx];
}

// Template specialization for ContainerNodeData
template <>
void RenderContext::ProcessChanges<ContainerNodeData>() {
	auto changes = m_changeBuffer.Snapshot<ContainerNodeData>();

	for (auto& c : changes) {
		if (c.deleted) {
			const std::uint64_t idx = ExtractIndex(c.id);
			m_nodeIdAllocator.Free(c.id);

			if (idx < m_containerGenerations.size()) {
				m_containerGenerations[idx] = m_nodeIdAllocator.GetGeneration(idx);
			}

			if (idx < m_renderContainers.size()) {
				m_renderContainers[idx] = RenderContainerNode{};
			}
		} else {
			c.Flush(*this);
		}
	}
}

// Template specialization for TextNodeData
template <>
void RenderContext::ProcessChanges<TextNodeData>() {
	auto changes = m_changeBuffer.Snapshot<TextNodeData>();

	for (auto& t : changes) {
		if (t.deleted) {
			const std::uint64_t idx = ExtractIndex(t.id);
			m_nodeIdAllocator.Free(t.id);

			if (idx < m_textGenerations.size()) {
				m_textGenerations[idx] = m_nodeIdAllocator.GetGeneration(idx);
			}

			if (idx < m_renderTexts.size()) {
				m_renderTexts[idx] = RenderTextNode{};
			}
		} else {
			t.Flush(*this);
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
    RenderContainerNode* r = render ? render : ctx.EnsureNode<ContainerNodeData>(id);
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
    RenderTextNode* r = render ? render : ctx.EnsureNode<TextNodeData>(id);
    render = r;
    r->x = x;
    r->y = y;
    r->visible = visible;
    r->text = text;
}

} // namespace ui
