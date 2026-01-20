#include "RenderContext.h"

#include "TraceProfiler.h"

#include <chrono>
#include <thread>

namespace ui {

RenderContainerNode* RenderContext::EnsureContainerNode(NodeId id) {
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

RenderTextNode* RenderContext::EnsureTextNode(NodeId id) {
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

RenderContainerNode* RenderContext::TryGetContainer(NodeId id) {
    const std::uint64_t idx = ExtractIndex(id);
    const std::uint16_t gen = ExtractGeneration(id);
    
    if (idx >= renderContainers_.size() || containerGenerations_[idx] != gen) {
        return nullptr;
    }
    
    return &renderContainers_[idx];
}

RenderTextNode* RenderContext::TryGetText(NodeId id) {
    const std::uint64_t idx = ExtractIndex(id);
    const std::uint16_t gen = ExtractGeneration(id);
    
    if (idx >= renderTexts_.size() || textGenerations_[idx] != gen) {
        return nullptr;
    }
    
    return &renderTexts_[idx];
}

ContainerNodeData& RenderContext::AccessContainerData(NodeId id) {
    return changeBuffer_.AccessContainerData(id);
}

TextNodeData& RenderContext::AccessTextData(NodeId id) {
    return changeBuffer_.AccessTextData(id);
}

void RenderContext::Sync() {
	std::lock_guard lock(renderMutex_);

    TRACE_SCOPE("RenderContext::Sync");

	auto containerChanges = changeBuffer_.SnapshotContainers();
	auto textChanges = changeBuffer_.SnapshotTexts();

	// Process container changes
	for (auto& c : containerChanges) {
		if (c.deleted) {
			// Node was deleted: free NodeId (increments generation) and clear render node
			const std::uint64_t idx = ExtractIndex(c.id);
			nodeIdAllocator_.Free(c.id);  // Increments generation, adds to free list
			
			// Sync generation to our local storage
			if (idx < containerGenerations_.size()) {
				containerGenerations_[idx] = nodeIdAllocator_.GetGeneration(idx);
			}
			
			// Clear render node
			if (idx < renderContainers_.size()) {
				renderContainers_[idx] = RenderContainerNode{};
			}
		} else {
			c.Flush(*this);
		}
	}
	
	// Process text changes
	for (auto& t : textChanges) {
		if (t.deleted) {
			// Node was deleted: free NodeId (increments generation) and clear render node
			const std::uint64_t idx = ExtractIndex(t.id);
			nodeIdAllocator_.Free(t.id);  // Increments generation, adds to free list
			
			// Sync generation to our local storage
			if (idx < textGenerations_.size()) {
				textGenerations_[idx] = nodeIdAllocator_.GetGeneration(idx);
			}
			
			// Clear render node
			if (idx < renderTexts_.size()) {
				renderTexts_[idx] = RenderTextNode{};
			}
		} else {
			t.Flush(*this);
		}
	}

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
}

void ContainerNodeData::Flush(RenderContext& ctx) {
    RenderContainerNode* r = render ? render : ctx.EnsureContainerNode(id);
    render = r;
    r->x = x;
    r->y = y;
    r->visible = visible;

    r->children.clear();
    r->children.reserve(children.size());
    for (const auto& child : children) {
        RenderChildPtr out;
        out.isText = child.isText;
        out.id = child.id;
        if (child.isText) {
            out.text = ctx.EnsureTextNode(child.id);
        } else {
            out.container = ctx.EnsureContainerNode(child.id);
        }
        r->children.push_back(out);
    }
}

void TextNodeData::Flush(RenderContext& ctx) {
    RenderTextNode* r = render ? render : ctx.EnsureTextNode(id);
    render = r;
    r->x = x;
    r->y = y;
    r->visible = visible;
    r->text = text;
}

} // namespace ui
