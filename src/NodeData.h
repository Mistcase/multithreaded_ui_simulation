#pragma once

#include "ui_ids.h"

#include <string>
#include <vector>

namespace ui {

class RenderContext;
struct RenderContainerNode;
struct RenderTextNode;
struct RenderShapeNode;
struct RenderShapeRectNode;

// ----------------------------
// Update-side (write) NodeData
// No virtuals and no base class hierarchy
// ----------------------------

struct ContainerNodeData {
    NodeId id{};
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
    bool deleted = false;  // Mark for deletion
    bool invalidateCommandsCache = false; // Invalidate render commands cache on flush
    std::vector<NodeId> children;
    RenderContainerNode* render = nullptr;

    void Flush(RenderContext& ctx);
};

struct TextNodeData {
    NodeId id{};
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
    bool deleted = false;  // Mark for deletion
    bool invalidateCommandsCache = false; // Invalidate render commands cache on flush
    std::string text;
    RenderTextNode* render = nullptr;

    void Flush(RenderContext& ctx);
};

struct ShapeNodeData {
    NodeId id{};
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
    bool deleted = false;  // Mark for deletion
    bool invalidateCommandsCache = false; // Invalidate render commands cache on flush
    RenderShapeNode* render = nullptr;

    void Flush(RenderContext& ctx);
};

struct ShapeRectNodeData {
    NodeId id{};
    float x = 0.0f;
    float y = 0.0f;
    bool visible = true;
    bool deleted = false;  // Mark for deletion
    bool invalidateCommandsCache = false; // Invalidate render commands cache on flush
    float width = 0.0f;
    float height = 0.0f;
    RenderShapeRectNode* render = nullptr;

    void Flush(RenderContext& ctx);
};

} // namespace ui
