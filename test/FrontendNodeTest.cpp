#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/FrontendNodes.h"
#include "../src/RenderContext.h"
#include "../src/ContainerNode.h"
#include "../src/TextNode.h"
#include "../src/ui_ids.h"

using namespace ui;

TEST_CASE("FrontendNode: Automatic cleanup in destructor", "[FrontendNode]") {
    RenderContext ctx;

    SECTION("Term() is automatically called on destruction") {
        NodeId id1 = ctx.AllocateNodeId();
        auto container = FrontendContainer::Create(id1, ctx);

        // Access data to verify it exists
        auto& containerData = ctx.AccessContainerData(id1);
        containerData.x = 10.0f;

        // Destroy frontend node
        container.reset();

        // Sync to process deletion
        ctx.Sync();

        // After sync, deleted node should be freed
        auto* renderContainer = ctx.TryGetContainer(id1);
        REQUIRE(renderContainer == nullptr);
    }

    SECTION("Backend node is marked as deleted in ChangeBuffer") {
        RenderContext ctx2;
        NodeId id1 = ctx2.AllocateNodeId();
        
        {
            auto container = FrontendContainer::Create(id1, ctx2);
            auto& containerData = ctx2.AccessContainerData(id1);
            containerData.x = 10.0f;
            
            // Container goes out of scope, destructor should call Term()
        }

        // Sync to process deletion
        ctx2.Sync();

        // Node should be deleted
        auto* renderContainer = ctx2.TryGetContainer(id1);
        REQUIRE(renderContainer == nullptr);
    }
}

TEST_CASE("FrontendNode: Explicit Term() call", "[FrontendNode]") {
    RenderContext ctx;

    SECTION("Term() frees backend") {
        NodeId id1 = ctx.AllocateNodeId();
        auto container = FrontendContainer::Create(id1, ctx);

        auto& containerData = ctx.AccessContainerData(id1);
        containerData.x = 10.0f;

        container->Term();

        ctx.Sync();

        // Node should be deleted
        auto* renderContainer = ctx.TryGetContainer(id1);
        REQUIRE(renderContainer == nullptr);
    }

    SECTION("Repeated Term() call is safe (idempotent)") {
        NodeId id1 = ctx.AllocateNodeId();
        auto container = FrontendContainer::Create(id1, ctx);

        container->Term();
        // Should not crash
        container->Term();
        container->Term();

        ctx.Sync();

        // Node should still be deleted (only once)
        auto* renderContainer = ctx.TryGetContainer(id1);
        REQUIRE(renderContainer == nullptr);
    }
}

TEST_CASE("FrontendNode: SetPosition/SetVisible after Term()", "[FrontendNode]") {
    RenderContext ctx;

    SECTION("Calls after Term() are safe") {
        NodeId id1 = ctx.AllocateNodeId();
        auto container = FrontendContainer::Create(id1, ctx);

        container->Term();

        // Should not crash
        container->SetPosition(10.0f, 20.0f);
        container->SetVisible(false);
        container->SetPosition(30.0f, 40.0f);

        ctx.Sync();
    }

    SECTION("SetPosition does nothing after Term()") {
        NodeId id1 = ctx.AllocateNodeId();
        auto container = FrontendContainer::Create(id1, ctx);

        container->SetPosition(10.0f, 20.0f);
        ctx.Sync();

        auto* renderContainer = ctx.TryGetContainer(id1);
        REQUIRE(renderContainer != nullptr);
        REQUIRE(renderContainer->x == 10.0f);
        REQUIRE(renderContainer->y == 20.0f);

        container->Term();
        container->SetPosition(100.0f, 200.0f); // Should be ignored

        ctx.Sync();

        // Position should not change (node is deleted anyway)
        auto* renderContainer2 = ctx.TryGetContainer(id1);
        REQUIRE(renderContainer2 == nullptr); // Node is deleted
    }

    SECTION("SetVisible does nothing after Term()") {
        NodeId id1 = ctx.AllocateNodeId();
        auto container = FrontendContainer::Create(id1, ctx);

        container->SetVisible(false);
        ctx.Sync();

        auto* renderContainer = ctx.TryGetContainer(id1);
        REQUIRE(renderContainer != nullptr);
        REQUIRE(renderContainer->visible == false);

        container->Term();
        container->SetVisible(true); // Should be ignored

        ctx.Sync();

        // Node is deleted anyway
        auto* renderContainer2 = ctx.TryGetContainer(id1);
        REQUIRE(renderContainer2 == nullptr);
    }
}

TEST_CASE("FrontendText: SetText after Term()", "[FrontendNode]") {
    RenderContext ctx;

    SECTION("SetText is safe after Term()") {
        NodeId id1 = ctx.AllocateNodeId();
        auto text = FrontendText::Create(id1, ctx);

        text->Term();

        // Should not crash
        text->SetText("Hello");
        text->SetText("World");

        ctx.Sync();
    }
}
