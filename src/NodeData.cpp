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
}

} // namespace ui
