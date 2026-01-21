#include "NodeData.h"
#include "RenderContext.h"

namespace ui {

void ContainerNodeData::Flush(RenderContext& ctx) {
    auto& renderContext = RenderContext::Instance();
    RenderContainerNode* r = render ? render : renderContext.EnsureRenderNode<ContainerNodeData>(id);
    render = r;
    r->x = x;
    r->y = y;
    r->visible = visible;
    if (invalidateCommandsCache) {
        r->isCommandsCacheValid = false;
        invalidateCommandsCache = false;
    }

    // Copy children - both use NodeId now
    r->children = children;
}

void TextNodeData::Flush(RenderContext& ctx) {
    auto& renderContext = RenderContext::Instance();
    RenderTextNode* r = render ? render : renderContext.EnsureRenderNode<TextNodeData>(id);
    render = r;
    r->x = x;
    r->y = y;
    r->visible = visible;
    r->text = text;
    if (invalidateCommandsCache) {
        r->isCommandsCacheValid = false;
        invalidateCommandsCache = false;
    }
}

void ShapeNodeData::Flush(RenderContext& ctx) {
    auto& renderContext = RenderContext::Instance();
    RenderShapeNode* r = render ? render : renderContext.EnsureRenderNode<ShapeNodeData>(id);
    render = r;
    r->x = x;
    r->y = y;
    r->visible = visible;
    if (invalidateCommandsCache) {
        invalidateCommandsCache = false;
    }
}

void ShapeRectNodeData::Flush(RenderContext& ctx) {
    auto& renderContext = RenderContext::Instance();
    RenderShapeRectNode* r = render ? render : renderContext.EnsureRenderNode<ShapeRectNodeData>(id);
    render = r;
    r->x = x;
    r->y = y;
    r->visible = visible;
    r->width = width;
    r->height = height;
    if (invalidateCommandsCache) {
        r->isCommandsCacheValid = false;
        invalidateCommandsCache = false;
    }
}

} // namespace ui
