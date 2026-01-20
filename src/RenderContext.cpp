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
    if (idx >= renderContainers_.size()) {
        renderContainers_.resize(idx + 1);
        containerGenerations_.resize(idx + 1, 0);
    }

    // Check generation match
    if (containerGenerations_[idx] != gen) {
        // Generation mismatch: reinitialize slot
        renderContainers_[idx] = RenderContainerNode{};
        containerGenerations_[idx] = gen;
    }

    return &renderContainers_[idx];
}

// Template specialization for TextNodeData
template <>
RenderTextNode* RenderContext::EnsureNode<TextNodeData>(NodeId id) {
    const std::uint64_t idx = ExtractIndex(id);
    const std::uint16_t gen = ExtractGeneration(id);

    // Expand vector if needed
    if (idx >= renderTexts_.size()) {
        renderTexts_.resize(idx + 1);
        textGenerations_.resize(idx + 1, 0);
    }

    // Check generation match
    if (textGenerations_[idx] != gen) {
        // Generation mismatch: reinitialize slot
        renderTexts_[idx] = RenderTextNode{};
        textGenerations_[idx] = gen;
    }

    return &renderTexts_[idx];
}

// Template specialization for ContainerNodeData
template <>
RenderContainerNode* RenderContext::TryGetNode<ContainerNodeData>(NodeId id) {
    const std::uint64_t idx = ExtractIndex(id);
    const std::uint16_t gen = ExtractGeneration(id);

    if (idx >= renderContainers_.size() || containerGenerations_[idx] != gen) {
        return nullptr;
    }

    return &renderContainers_[idx];
}

// Template specialization for TextNodeData
template <>
RenderTextNode* RenderContext::TryGetNode<TextNodeData>(NodeId id) {
    const std::uint64_t idx = ExtractIndex(id);
    const std::uint16_t gen = ExtractGeneration(id);

    if (idx >= renderTexts_.size() || textGenerations_[idx] != gen) {
        return nullptr;
    }

    return &renderTexts_[idx];
}

// Template specialization for ContainerNodeData
template <>
void RenderContext::ProcessChanges<ContainerNodeData>() {
	auto changes = changeBuffer_.Snapshot<ContainerNodeData>();

	for (auto& c : changes) {
		if (c.deleted) {
			const std::uint64_t idx = ExtractIndex(c.id);
			nodeIdAllocator_.Free(c.id);

			if (idx < containerGenerations_.size()) {
				containerGenerations_[idx] = nodeIdAllocator_.GetGeneration(idx);
			}

			if (idx < renderContainers_.size()) {
				renderContainers_[idx] = RenderContainerNode{};
			}
		} else {
			c.Flush(*this);
		}
	}
}

// Template specialization for TextNodeData
template <>
void RenderContext::ProcessChanges<TextNodeData>() {
	auto changes = changeBuffer_.Snapshot<TextNodeData>();

	for (auto& t : changes) {
		if (t.deleted) {
			const std::uint64_t idx = ExtractIndex(t.id);
			nodeIdAllocator_.Free(t.id);

			if (idx < textGenerations_.size()) {
				textGenerations_[idx] = nodeIdAllocator_.GetGeneration(idx);
			}

			if (idx < renderTexts_.size()) {
				renderTexts_[idx] = RenderTextNode{};
			}
		} else {
			t.Flush(*this);
		}
	}
}

void RenderContext::Sync() {
	std::lock_guard lock(renderMutex_);

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
