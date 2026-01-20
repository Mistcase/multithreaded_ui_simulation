#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/ui_ids.h"

using namespace ui;

TEST_CASE("NodeId: ExtractIndex/ExtractGeneration", "[NodeId]") {
    SECTION("Correctly extract components from NodeId") {
        NodeId id1 = MakeNodeId(0, 0);
        REQUIRE(ExtractIndex(id1) == 0);
        REQUIRE(ExtractGeneration(id1) == 0);

        NodeId id2 = MakeNodeId(5, 10);
        REQUIRE(ExtractIndex(id2) == 5);
        REQUIRE(ExtractGeneration(id2) == 10);

        NodeId id3 = MakeNodeId(100, 255);
        REQUIRE(ExtractIndex(id3) == 100);
        REQUIRE(ExtractGeneration(id3) == 255);
    }

    SECTION("Works with boundary values") {
        // Maximum index (64-bit)
        NodeId id1 = MakeNodeId(UINT64_MAX >> 16, 0);
        std::uint64_t maxIndex = UINT64_MAX >> 16;
        REQUIRE(ExtractIndex(id1) == maxIndex);

        // Maximum generation (16-bit)
        NodeId id2 = MakeNodeId(0, 0xFFFF);
        REQUIRE(ExtractGeneration(id2) == 0xFFFF);

        // Both maximum
        NodeId id3 = MakeNodeId(maxIndex, 0xFFFF);
        REQUIRE(ExtractIndex(id3) == maxIndex);
        REQUIRE(ExtractGeneration(id3) == 0xFFFF);
    }

    SECTION("ExtractIndex handles large values") {
        NodeId id1 = MakeNodeId(1000, 0);
        REQUIRE(ExtractIndex(id1) == 1000);

        NodeId id2 = MakeNodeId(100000, 0);
        REQUIRE(ExtractIndex(id2) == 100000);

        NodeId id3 = MakeNodeId(50000, 0);
        REQUIRE(ExtractIndex(id3) == 50000);
    }

    SECTION("ExtractGeneration handles all 16-bit values") {
        for (std::uint16_t gen = 0; gen < 100; ++gen) {
            NodeId id = MakeNodeId(0, gen);
            REQUIRE(ExtractGeneration(id) == gen);
        }

        // Test boundary
        NodeId idMax = MakeNodeId(0, 0xFFFF);
        REQUIRE(ExtractGeneration(idMax) == 0xFFFF);
    }
}

TEST_CASE("NodeId: MakeNodeId", "[NodeId]") {
    SECTION("Correctly creates NodeId from index and generation") {
        NodeId id1 = MakeNodeId(0, 0);
        REQUIRE(ExtractIndex(id1) == 0);
        REQUIRE(ExtractGeneration(id1) == 0);

        NodeId id2 = MakeNodeId(42, 7);
        REQUIRE(ExtractIndex(id2) == 42);
        REQUIRE(ExtractGeneration(id2) == 7);

        NodeId id3 = MakeNodeId(1000, 255);
        REQUIRE(ExtractIndex(id3) == 1000);
        REQUIRE(ExtractGeneration(id3) == 255);
    }

    SECTION("Reversibility: ExtractIndex(MakeNodeId(i, g)) == i") {
        for (std::uint64_t i = 0; i < 1000; i += 100) {
            for (std::uint16_t g = 0; g < 100; g += 10) {
                NodeId id = MakeNodeId(i, g);
                REQUIRE(ExtractIndex(id) == i);
                REQUIRE(ExtractGeneration(id) == g);
            }
        }
    }

    SECTION("Reversibility: ExtractGeneration(MakeNodeId(i, g)) == g") {
        for (std::uint64_t i = 0; i < 100; ++i) {
            for (std::uint16_t g = 0; g < 100; ++g) {
                NodeId id = MakeNodeId(i, g);
                REQUIRE(ExtractGeneration(id) == g);
            }
        }
    }

    SECTION("Index and generation are independent") {
        NodeId id1 = MakeNodeId(10, 5);
        NodeId id2 = MakeNodeId(10, 6);
        NodeId id3 = MakeNodeId(11, 5);

        // Same index, different generation
        REQUIRE(ExtractIndex(id1) == ExtractIndex(id2));
        REQUIRE(ExtractGeneration(id1) != ExtractGeneration(id2));

        // Same generation, different index
        REQUIRE(ExtractIndex(id1) != ExtractIndex(id3));
        REQUIRE(ExtractGeneration(id1) == ExtractGeneration(id3));

        // All different
        REQUIRE(id1 != id2);
        REQUIRE(id1 != id3);
        REQUIRE(id2 != id3);
    }

    SECTION("Handles edge cases correctly") {
        // Zero values
        NodeId id1 = MakeNodeId(0, 0);
        REQUIRE(id1 == 0);
        REQUIRE(ExtractIndex(id1) == 0);
        REQUIRE(ExtractGeneration(id1) == 0);

        // Large index, small generation
        NodeId id2 = MakeNodeId(1000000, 1);
        REQUIRE(ExtractIndex(id2) == 1000000);
        REQUIRE(ExtractGeneration(id2) == 1);

        // Small index, large generation
        NodeId id3 = MakeNodeId(1, 0xFFFF);
        REQUIRE(ExtractIndex(id3) == 1);
        REQUIRE(ExtractGeneration(id3) == 0xFFFF);
    }
}
