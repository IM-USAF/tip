#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tmats_data.h"

TEST(TMATSDataTest, CombineMaps)
{
    TMATSData td;

    cmapmap map1 = {
        {"1", {{"2", "a"}, {"3", "b"}}},
        {"12", {{"22", "aa"}, {"33", "bb"}}}
    };
    
    cmapmap map2 = {
        {"1", {{"2", "A"}, {"3", "B"}}}
    };

    cmapmap expected = {
        {"1", {{"a", "A"}, {"b", "B"}}}
    };

    cmapmap result;
    td.CombineMaps(map1, map2, result);

    EXPECT_THAT(expected, ::testing::ContainerEq(result));
}

TEST(TMATSDataTest, FilterTMATSType)
{
    TMATSData td;

    cmap type_map = {
        {"23", "1553IN"},
        {"24", "429IN"},
        {"27", "1553IN"}
    };

    cmap expected1 = {
        {"23", "1553IN"},
        {"27", "1553IN"}
    };

    cmap filtered_map;
    EXPECT_TRUE(td.FilterTMATSType(type_map, Ch10PacketType::MILSTD1553_F1, filtered_map));
    EXPECT_THAT(expected1, ::testing::ContainerEq(filtered_map));

    cmap expected2 = {
        {"24", "429IN"}
    };
    filtered_map.clear();
    EXPECT_TRUE(td.FilterTMATSType(type_map, Ch10PacketType::ARINC429_F0, filtered_map));
    EXPECT_THAT(expected2, ::testing::ContainerEq(filtered_map));

    cmap expected3 = {};
    filtered_map.clear();
    EXPECT_TRUE(td.FilterTMATSType(type_map, Ch10PacketType::VIDEO_DATA_F0, filtered_map));
    EXPECT_THAT(expected3, ::testing::ContainerEq(filtered_map));
}

TEST(TMATSDataTest, FilterTMATSTypeNotInMap)
{
    TMATSData td;

    cmap type_map = {
        {"23", "1553IN"},
        {"24", "429IN"},
        {"27", "1553IN"}
    };

    cmap filtered_map;
    ASSERT_FALSE(td.FilterTMATSType(type_map, Ch10PacketType::NONE, filtered_map));
    EXPECT_TRUE(filtered_map.size() == 0);
}

TEST(TMATSDataTest, FilterByChannelIDToType)
{
    TMATSData td;

    cmap type_map = {
        {"23", "1553IN"},
        {"27", "1553IN"}
    };

    cmap input_map = {
        {"28", "data1"},
        {"29", "data2"},
        {"27", "data3"}
    };

    cmap expected = {
        {"27", "data3"}
    };

    cmap filtered_map;
    filtered_map = td.FilterByChannelIDToType(type_map, input_map);
    EXPECT_THAT(expected, ::testing::ContainerEq(filtered_map));
}
