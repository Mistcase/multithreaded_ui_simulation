#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/ChangeBuffer.h"
#include "../src/RenderContext.h"
#include "../src/ui_ids.h"

using namespace ui;

// Mock RenderContext for testing ChangeBuffer in isolation
class MockRenderContext {
public:
    ContainerNodeData& AccessContainerData(NodeId id) {
        return changeBuffer_.AccessContainerData(id);
    }

    TextNodeData& AccessTextData(NodeId id) {
        return changeBuffer_.AccessTextData(id);
    }

    std::size_t Version() const {
        return changeBuffer_.Version();
    }

    std::vector<ContainerNodeData> SnapshotContainers() {
        return changeBuffer_.SnapshotContainers();
    }

    std::vector<TextNodeData> SnapshotTexts() {
        return changeBuffer_.SnapshotTexts();
    }

    bool Empty() const {
        return changeBuffer_.Empty();
    }

private:
    ChangeBuffer changeBuffer_;
};

TEST_CASE("ChangeBuffer: Basic batching", "[ChangeBuffer]") {
    MockRenderContext ctx;

    SECTION("AccessContainerData() creates entry in buffer") {
        NodeId id1 = MakeNodeId(0, 0);
        NodeId id2 = MakeNodeId(1, 0);

        auto& data1 = ctx.AccessContainerData(id1);
        auto& data2 = ctx.AccessContainerData(id2);

        REQUIRE(data1.id == id1);
        REQUIRE(data2.id == id2);
        REQUIRE(!ctx.Empty());
    }

    SECTION("AccessTextData() creates entry in buffer") {
        NodeId id1 = MakeNodeId(0, 0);
        NodeId id2 = MakeNodeId(1, 0);

        auto& data1 = ctx.AccessTextData(id1);
        auto& data2 = ctx.AccessTextData(id2);

        REQUIRE(data1.id == id1);
        REQUIRE(data2.id == id2);
        REQUIRE(!ctx.Empty());
    }

    SECTION("Empty() returns false after access") {
        REQUIRE(ctx.Empty());

        NodeId id1 = MakeNodeId(0, 0);
        ctx.AccessContainerData(id1);

        REQUIRE(!ctx.Empty());
    }
}

TEST_CASE("ChangeBuffer: Multiple changes to same node", "[ChangeBuffer]") {
    MockRenderContext ctx;

    SECTION("Multiple AccessData() calls for same NodeId return same entry") {
        NodeId id1 = MakeNodeId(0, 0);

        auto& data1 = ctx.AccessContainerData(id1);
        data1.x = 10.0f;
        data1.y = 20.0f;

        auto& data2 = ctx.AccessContainerData(id1);
        data2.x = 30.0f;

        // Should be the same reference
        REQUIRE(&data1 == &data2);
        REQUIRE(data1.x == 30.0f);
        REQUIRE(data1.y == 20.0f); // y should still be set
    }

    SECTION("Changes accumulate in single entry") {
        NodeId id1 = MakeNodeId(0, 0);

        auto& data1 = ctx.AccessContainerData(id1);
        data1.x = 10.0f;

        auto& data2 = ctx.AccessContainerData(id1);
        data2.y = 20.0f;

        auto& data3 = ctx.AccessContainerData(id1);
        data3.visible = false;

        REQUIRE(data1.x == 10.0f);
        REQUIRE(data1.y == 20.0f);
        REQUIRE(data1.visible == false);
    }
}

TEST_CASE("ChangeBuffer: SnapshotAndClear", "[ChangeBuffer]") {
    MockRenderContext ctx;

    SECTION("SnapshotContainers() returns all modified containers") {
        NodeId id1 = MakeNodeId(0, 0);
        NodeId id2 = MakeNodeId(1, 0);
        NodeId id3 = MakeNodeId(2, 0);

        auto& data1 = ctx.AccessContainerData(id1);
        data1.x = 10.0f;

        auto& data2 = ctx.AccessContainerData(id2);
        data2.y = 20.0f;

        auto& data3 = ctx.AccessContainerData(id3);
        data3.visible = false;

        auto snapshot = ctx.SnapshotContainers();

        REQUIRE(snapshot.size() == 3);
        REQUIRE((snapshot[0].id == id1 || snapshot[1].id == id1 || snapshot[2].id == id1));
        REQUIRE((snapshot[0].id == id2 || snapshot[1].id == id2 || snapshot[2].id == id2));
        REQUIRE((snapshot[0].id == id3 || snapshot[1].id == id3 || snapshot[2].id == id3));
    }

    SECTION("SnapshotTexts() returns all modified texts") {
        NodeId id1 = MakeNodeId(0, 0);
        NodeId id2 = MakeNodeId(1, 0);

        auto& data1 = ctx.AccessTextData(id1);
        data1.text = "Hello";

        auto& data2 = ctx.AccessTextData(id2);
        data2.text = "World";

        auto snapshot = ctx.SnapshotTexts();

        REQUIRE(snapshot.size() == 2);
    }

    SECTION("After Snapshot* buffer is cleared (Empty() = true)") {
        NodeId id1 = MakeNodeId(0, 0);
        ctx.AccessContainerData(id1);

        REQUIRE(!ctx.Empty());

        ctx.SnapshotContainers();

        REQUIRE(ctx.Empty());
    }

    SECTION("Version increments after each snapshot") {
        std::size_t version1 = ctx.Version();

        NodeId id1 = MakeNodeId(0, 0);
        ctx.AccessContainerData(id1);
        ctx.SnapshotContainers();

        std::size_t version2 = ctx.Version();
        REQUIRE(version2 == version1 + 1);

        ctx.AccessContainerData(id1);
        ctx.SnapshotContainers();

        std::size_t version3 = ctx.Version();
        REQUIRE(version3 == version2 + 1);
    }
}

TEST_CASE("ChangeBuffer: Direct index access", "[ChangeBuffer]") {
    MockRenderContext ctx;

    SECTION("AccessData() uses ExtractIndex(id) for direct access") {
        NodeId id1 = MakeNodeId(5, 0);
        NodeId id2 = MakeNodeId(10, 0);
        NodeId id3 = MakeNodeId(100, 0);

        auto& data1 = ctx.AccessContainerData(id1);
        auto& data2 = ctx.AccessContainerData(id2);
        auto& data3 = ctx.AccessContainerData(id3);

        REQUIRE(data1.id == id1);
        REQUIRE(data2.id == id2);
        REQUIRE(data3.id == id3);
    }

    SECTION("Works correctly with large indices (sparse)") {
        NodeId id1 = MakeNodeId(0, 0);
        NodeId id2 = MakeNodeId(1000, 0);
        NodeId id3 = MakeNodeId(50000, 0);

        auto& data1 = ctx.AccessContainerData(id1);
        auto& data2 = ctx.AccessContainerData(id2);
        auto& data3 = ctx.AccessContainerData(id3);

        REQUIRE(data1.id == id1);
        REQUIRE(data2.id == id2);
        REQUIRE(data3.id == id3);

        auto snapshot = ctx.SnapshotContainers();
        REQUIRE(snapshot.size() == 3);
    }

    SECTION("dirty_ flag correctly tracks modified indices") {
        NodeId id1 = MakeNodeId(0, 0);

        // First access should mark as dirty
        ctx.AccessContainerData(id1);
        REQUIRE(!ctx.Empty());

        // Snapshot should clear dirty flag
        ctx.SnapshotContainers();
        REQUIRE(ctx.Empty());

        // Second access should create new entry
        ctx.AccessContainerData(id1);
        REQUIRE(!ctx.Empty());
    }
}

TEST_CASE("ChangeBuffer: Edge cases", "[ChangeBuffer]") {
    MockRenderContext ctx;

    SECTION("Access to same NodeId multiple times in frame") {
        NodeId id1 = MakeNodeId(0, 0);

        auto& data1 = ctx.AccessContainerData(id1);
        auto& data2 = ctx.AccessContainerData(id1);
        auto& data3 = ctx.AccessContainerData(id1);

        REQUIRE(&data1 == &data2);
        REQUIRE(&data2 == &data3);

        // Should only appear once in snapshot
        auto snapshot = ctx.SnapshotContainers();
        REQUIRE(snapshot.size() == 1);
    }

    SECTION("SnapshotAndClear() returns only modified elements") {
        NodeId id1 = MakeNodeId(0, 0);
        NodeId id2 = MakeNodeId(1, 0);

        ctx.AccessContainerData(id1);
        // Don't access id2

        auto snapshot = ctx.SnapshotContainers();
        REQUIRE(snapshot.size() == 1);
        REQUIRE(snapshot[0].id == id1);
    }

    SECTION("After SnapshotAndClear() repeated access creates new entry") {
        NodeId id1 = MakeNodeId(0, 0);

        auto& data1 = ctx.AccessContainerData(id1);
        data1.x = 10.0f;

        auto snapshot1 = ctx.SnapshotContainers();
        REQUIRE(snapshot1[0].x == 10.0f);

        // Access again after snapshot
        auto& data2 = ctx.AccessContainerData(id1);
        data2.x = 20.0f;

        auto snapshot2 = ctx.SnapshotContainers();
        REQUIRE(snapshot2[0].x == 20.0f);
    }
}
