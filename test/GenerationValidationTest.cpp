#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/RenderContext.h"
#include "../src/ui_ids.h"

using namespace ui;

TEST_CASE("Generation Validation: TryGetContainer/TryGetText with valid handle", "[GenerationValidation]") {
    RenderContext ctx;

    SECTION("Returns pointer for valid NodeId") {
        NodeId id1 = ctx.AllocateNodeId();
        NodeId id2 = ctx.AllocateNodeId();

        // Access data to create render nodes
        auto& containerData = ctx.AccessContainerData(id1);
        containerData.x = 10.0f;
        containerData.y = 20.0f;

        auto& textData = ctx.AccessTextData(id2);
        textData.text = "Hello";

        ctx.Sync();

        // Should be able to retrieve
        auto* container = ctx.TryGetContainer(id1);
        REQUIRE(container != nullptr);
        REQUIRE(container->x == 10.0f);
        REQUIRE(container->y == 20.0f);

        auto* text = ctx.TryGetText(id2);
        REQUIRE(text != nullptr);
        REQUIRE(text->text == "Hello");
    }

    SECTION("Checks generation match") {
        NodeId id1 = ctx.AllocateNodeId();

        auto& containerData = ctx.AccessContainerData(id1);
        containerData.x = 10.0f;

        ctx.Sync();

        auto* container = ctx.TryGetContainer(id1);
        REQUIRE(container != nullptr);
        REQUIRE(container->x == 10.0f);
    }
}

TEST_CASE("Generation Validation: TryGetContainer/TryGetText with invalid handle", "[GenerationValidation]") {
    RenderContext ctx;

    SECTION("Returns nullptr for deleted node (old generation)") {
        NodeId id1 = ctx.AllocateNodeId();

        auto& containerData = ctx.AccessContainerData(id1);
        containerData.x = 10.0f;

        ctx.Sync();

        // Verify it exists
        auto* container1 = ctx.TryGetContainer(id1);
        REQUIRE(container1 != nullptr);

        // Delete the node
        containerData.deleted = true;
        ctx.Sync();

        // Old handle should be invalid
        auto* container2 = ctx.TryGetContainer(id1);
        REQUIRE(container2 == nullptr);
    }

    SECTION("Returns nullptr for non-existent index") {
        NodeId invalidId = MakeNodeId(99999, 0);

        auto* container = ctx.TryGetContainer(invalidId);
        REQUIRE(container == nullptr);

        auto* text = ctx.TryGetText(invalidId);
        REQUIRE(text == nullptr);
    }

    SECTION("Returns nullptr for handle with mismatched generation") {
        NodeId id1 = ctx.AllocateNodeId();
        std::uint64_t idx1 = ExtractIndex(id1);
        std::uint16_t gen1 = ExtractGeneration(id1);

        auto& containerData = ctx.AccessContainerData(id1);
        containerData.x = 10.0f;

        ctx.Sync();

        // Create handle with wrong generation
        NodeId wrongGenId = MakeNodeId(idx1, gen1 + 1);

        auto* container = ctx.TryGetContainer(wrongGenId);
        REQUIRE(container == nullptr);
    }

    SECTION("Text node deletion invalidates handle") {
        NodeId id1 = ctx.AllocateNodeId();

        auto& textData = ctx.AccessTextData(id1);
        textData.text = "Hello";

        ctx.Sync();

        // Verify it exists
        auto* text1 = ctx.TryGetText(id1);
        REQUIRE(text1 != nullptr);
        REQUIRE(text1->text == "Hello");

        // Delete the node
        textData.deleted = true;
        ctx.Sync();

        // Old handle should be invalid
        auto* text2 = ctx.TryGetText(id1);
        REQUIRE(text2 == nullptr);
    }
}

TEST_CASE("Generation Validation: EnsureContainerNode/EnsureTextNode", "[GenerationValidation]") {
    RenderContext ctx;

    SECTION("Creates new slot if not exists") {
        NodeId id1 = MakeNodeId(5, 0);

        auto* container = ctx.EnsureContainerNode(id1);
        REQUIRE(container != nullptr);

        auto* text = ctx.EnsureTextNode(id1);
        REQUIRE(text != nullptr);
    }

    SECTION("Reinitializes slot on generation mismatch") {
        NodeId id1 = ctx.AllocateNodeId();
        std::uint64_t idx1 = ExtractIndex(id1);

        // Create initial node
        auto& containerData = ctx.AccessContainerData(id1);
        containerData.x = 10.0f;
        containerData.y = 20.0f;

        ctx.Sync();

        // Delete and free
        containerData.deleted = true;
        ctx.Sync();

        // Create new node with same index but new generation
        NodeId id2 = ctx.AllocateNodeId();
        // If free list is used, id2 might reuse idx1
        // But if not, we need to manually test with same index

        // Manually test with same index but new generation
        NodeId newId = MakeNodeId(idx1, 1); // Assuming generation was incremented
        auto* container = ctx.EnsureContainerNode(newId);
        REQUIRE(container != nullptr);
        // Should be reinitialized (default values)
        REQUIRE(container->x == 0.0f);
        REQUIRE(container->y == 0.0f);
    }

    SECTION("Updates generation in local storage") {
        NodeId id1 = ctx.AllocateNodeId();
        std::uint64_t idx1 = ExtractIndex(id1);

        auto* container1 = ctx.EnsureContainerNode(id1);
        REQUIRE(container1 != nullptr);

        // After deletion and reallocation, generation should be updated
        auto& containerData = ctx.AccessContainerData(id1);
        containerData.deleted = true;
        ctx.Sync();

        // New node with incremented generation
        NodeId newId = MakeNodeId(idx1, 1);
        auto* container2 = ctx.EnsureContainerNode(newId);
        REQUIRE(container2 != nullptr);
    }
}
