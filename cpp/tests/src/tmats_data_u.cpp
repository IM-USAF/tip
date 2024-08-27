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

TEST(TMATSDataTest, PopulatePCMDataObject)
{
    std::map<std::string, std::string> code_to_vals{
        {"P-d\\DLN", "link name"},
        {"P-d\\D1", "pcmcode"},
        {"P-d\\D2", "12.5"},
        {"P-d\\D3", "encrypted"},
        {"P-d\\D4", "polarity"},
        {"P-d\\D5", "polcorrection"}, 
        {"P-d\\D6", "datadir"},
        {"P-d\\D7", "datarando"}, 
        {"P-d\\D8", "randotype"}, 
        {"P-d\\TF", "typeformat"}, 
        {"P-d\\F1", "32"},
        {"P-d\\F2", "transforder"},
        {"P-d\\F3", "parity"}, 
        {"P-d\\F4", "parityorder"}, 
        {"P-d\\CRC", "crc"}, 
        {"P-d\\CRCCB", "21"},
        {"P-d\\CRCDB", "36"}, 
        {"P-d\\CRCDN", "10"}, 
        {"P-d\\MF\\N", "7"}, 
        {"P-d\\MF1", "28"}, 
        {"P-d\\MF2", "384"},
        {"P-d\\MF3", "synctype"}
    };

    TMATSData td;
    Ch10PCMTMATSData pcmdata;
    ASSERT_TRUE(td.PopulatePCMDataObject(code_to_vals, pcmdata));
    ASSERT_EQ("link name", pcmdata.data_link_name_);
    ASSERT_EQ("pcmcode", pcmdata.pcm_code_);
    ASSERT_EQ(12.5, pcmdata.bit_rate_);
    ASSERT_EQ("encrypted", pcmdata.encrypted_);
    ASSERT_EQ("polarity", pcmdata.polarity_);
    ASSERT_EQ("polcorrection", pcmdata.auto_pol_correction_);
    ASSERT_EQ("datadir", pcmdata.data_direction_);
    ASSERT_EQ("datarando", pcmdata.data_randomized_);
    ASSERT_EQ("randotype", pcmdata.randomizer_type_);
    ASSERT_EQ("typeformat", pcmdata.type_format_);
    ASSERT_EQ(32, pcmdata.common_word_length_);
    ASSERT_EQ("transforder", pcmdata.word_transfer_order_);
    ASSERT_EQ("parity", pcmdata.parity_);
    ASSERT_EQ("parityorder", pcmdata.parity_transfer_order_);
    ASSERT_EQ("crc", pcmdata.crc_);
    ASSERT_EQ(21, pcmdata.crc_check_word_starting_bit_);
    ASSERT_EQ(36, pcmdata.crc_data_start_bit_);
    ASSERT_EQ(10, pcmdata.crc_data_number_of_bits_);
    ASSERT_EQ(7, pcmdata.min_frames_in_maj_frame_);
    ASSERT_EQ(28, pcmdata.words_in_min_frame_);
    ASSERT_EQ(384, pcmdata.bits_in_min_frame_);
    ASSERT_EQ("synctype", pcmdata.sync_type_);
}

TEST(TMATSDataTest, PopulatePCMDataObjectNullHandling)
{
    std::map<std::string, std::string> code_to_vals{
        {"P-d\\DLN", "link name"},
        {"P-d\\D1", "pcmcode"},
        // {"P-d\\D2", "12.5"},  // omit
        {"P-d\\D3", "encrypted"},
        {"P-d\\D4", "polarity"},
        {"P-d\\D5", "polcorrection"}, 
        {"P-d\\D6", "datadir"},
        // {"P-d\\D7", "datarando"},  // omit
        {"P-d\\D8", "randotype"}, 
        {"P-d\\TF", "typeformat"}, 
        // {"P-d\\F1", "32"},  // omit 
        {"P-d\\F2", "transforder"},
        {"P-d\\F3", "parity"}, 
        {"P-d\\F4", "parityorder"}, 
        // {"P-d\\CRC", "crc"},   // omit
        {"P-d\\CRCCB", "21"},
        {"P-d\\CRCDB", "36"}, 
        {"P-d\\CRCDN", "10"}, 
        {"P-d\\MF\\N", "7"}, 
        {"P-d\\MF1", "28"}, 
        {"P-d\\MF2", "384"},
        {"P-d\\MF3", "synctype"}
    };

    TMATSData td;
    Ch10PCMTMATSData pcmdata;
    ASSERT_TRUE(td.PopulatePCMDataObject(code_to_vals, pcmdata));
    ASSERT_EQ("link name", pcmdata.data_link_name_);
    ASSERT_EQ("pcmcode", pcmdata.pcm_code_);
    ASSERT_EQ(-1.0, pcmdata.bit_rate_);
    ASSERT_EQ("encrypted", pcmdata.encrypted_);
    ASSERT_EQ("polarity", pcmdata.polarity_);
    ASSERT_EQ("polcorrection", pcmdata.auto_pol_correction_);
    ASSERT_EQ("datadir", pcmdata.data_direction_);
    ASSERT_EQ("null", pcmdata.data_randomized_);
    ASSERT_EQ("randotype", pcmdata.randomizer_type_);
    ASSERT_EQ("typeformat", pcmdata.type_format_);
    ASSERT_EQ(-1, pcmdata.common_word_length_);
    ASSERT_EQ("transforder", pcmdata.word_transfer_order_);
    ASSERT_EQ("parity", pcmdata.parity_);
    ASSERT_EQ("parityorder", pcmdata.parity_transfer_order_);
    ASSERT_EQ("null", pcmdata.crc_);
    ASSERT_EQ(21, pcmdata.crc_check_word_starting_bit_);
    ASSERT_EQ(36, pcmdata.crc_data_start_bit_);
    ASSERT_EQ(10, pcmdata.crc_data_number_of_bits_);
    ASSERT_EQ(7, pcmdata.min_frames_in_maj_frame_);
    ASSERT_EQ(28, pcmdata.words_in_min_frame_);
    ASSERT_EQ(384, pcmdata.bits_in_min_frame_);
    ASSERT_EQ("synctype", pcmdata.sync_type_);
}

TEST(TMATSDataTest, PopulatePCMDataObjectCastErrorFloat)
{
    std::map<std::string, std::string> code_to_vals{
        {"P-d\\DLN", "link name"},
        {"P-d\\D1", "pcmcode"},
        {"P-d\\D2", "not float!"}, // mod from 12.5
        {"P-d\\D3", "encrypted"},
        {"P-d\\D4", "polarity"},
        {"P-d\\D5", "polcorrection"}, 
        {"P-d\\D6", "datadir"},
        {"P-d\\D7", "datarando"}, 
        {"P-d\\D8", "randotype"}, 
        {"P-d\\TF", "typeformat"}, 
        {"P-d\\F1", "32"},
        {"P-d\\F2", "transforder"},
        {"P-d\\F3", "parity"}, 
        {"P-d\\F4", "parityorder"}, 
        {"P-d\\CRC", "crc"}, 
        {"P-d\\CRCCB", "21"},
        {"P-d\\CRCDB", "36"}, 
        {"P-d\\CRCDN", "10"}, 
        {"P-d\\MF\\N", "7"}, 
        {"P-d\\MF1", "28"}, 
        {"P-d\\MF2", "384"},
        {"P-d\\MF3", "synctype"}
    };

    TMATSData td;
    Ch10PCMTMATSData pcmdata;
    ASSERT_FALSE(td.PopulatePCMDataObject(code_to_vals, pcmdata));
}

TEST(TMATSDataTest, PopulatePCMDataObjectCastErrorInt)
{
    std::map<std::string, std::string> code_to_vals{
        {"P-d\\DLN", "link name"},
        {"P-d\\D1", "pcmcode"},
        {"P-d\\D2", "12.5"}, 
        {"P-d\\D3", "encrypted"},
        {"P-d\\D4", "polarity"},
        {"P-d\\D5", "polcorrection"}, 
        {"P-d\\D6", "datadir"},
        {"P-d\\D7", "datarando"}, 
        {"P-d\\D8", "randotype"}, 
        {"P-d\\TF", "typeformat"}, 
        {"P-d\\F1", "32"},
        {"P-d\\F2", "transforder"},
        {"P-d\\F3", "parity"}, 
        {"P-d\\F4", "parityorder"}, 
        {"P-d\\CRC", "crc"}, 
        {"P-d\\CRCCB", "21"},
        {"P-d\\CRCDB", "36"}, 
        {"P-d\\CRCDN", "not int!"}, // mod from 10
        {"P-d\\MF\\N", "7"}, 
        {"P-d\\MF1", "28"}, 
        {"P-d\\MF2", "384"},
        {"P-d\\MF3", "synctype"}
    };

    TMATSData td;
    Ch10PCMTMATSData pcmdata;
    ASSERT_FALSE(td.PopulatePCMDataObject(code_to_vals, pcmdata));
}

TEST(TMATSDataTest, ParsePCMAttributesParseFailMissingReq)
{
    std::string tmats =  
        R"(P-1\F1:16;)"
        "\n"
        R"(P-1\MF1:101;)"
        "\n"
        R"(P-1\MF2:201;)"
        "\n"
        R"(P-4\MF2:204;)"
        "\n"
        R"(P-1\MF\N:31;)"
        "\n"
        // R"(P-4\MF\N:34;)"  // required
        // "\n"
        R"(P-4\MF1:104;)"
        "\n"
        R"(P-1\TF:tf;)"
        "\n"
        R"(P-4\F1:20;)"
        "\n"
        R"(P-4\DLN:pcm4;)"
        "\n"
        R"(P-1\D4:N;)"
        "\n"
        R"(P-4\TF:tf4;)"
        "\n"
        R"(P-1\DLN:pcm1;)"
        "\n";
    TMATSParser parser(tmats);
    pcmdata_map output;
    TMATSData td;
    ASSERT_FALSE(td.ParsePCMAttributes(parser, output));
}

TEST(TMATSDataTest, ParsePCMAttributesCastError)
{
    std::string tmats =  
        R"(P-1\F1:16;)"
        "\n"
        R"(P-1\MF1:101;)"
        "\n"
        R"(P-1\MF2:201;)"
        "\n"
        R"(P-4\MF2:204;)"
        "\n"
        R"(P-1\MF\N:oh no!;)"  // must be castable to int
        "\n"
        R"(P-4\MF\N:34;)" 
        "\n"
        R"(P-4\MF1:104;)"
        "\n"
        R"(P-1\TF:tf;)"
        "\n"
        R"(P-4\F1:20;)"
        "\n"
        R"(P-4\DLN:pcm4;)"
        "\n"
        R"(P-1\D4:N;)"
        "\n"
        R"(P-4\TF:tf4;)"
        "\n"
        R"(P-1\DLN:pcm1;)"
        "\n";
    TMATSParser parser(tmats);
    pcmdata_map output;
    TMATSData td;
    ASSERT_FALSE(td.ParsePCMAttributes(parser, output));
}

TEST(TMATSDataTest, ParsePCMAttributes)
{
    std::string tmats =  
        R"(P-1\F1:16;)"
        "\n"
        R"(P-1\MF1:101;)"
        "\n"
        R"(P-1\MF2:201;)"
        "\n"
        R"(P-4\MF2:204;)"
        "\n"
        R"(P-1\MF\N:31;)"  
        "\n"
        R"(P-4\MF\N:34;)" 
        "\n"
        R"(P-4\MF1:104;)"
        "\n"
        R"(P-1\TF:tf;)"
        "\n"
        R"(P-4\F1:20;)"
        "\n"
        R"(P-4\DLN:pcm4;)"
        "\n"
        R"(P-1\D4:N;)"
        "\n"
        R"(P-4\TF:tf4;)"
        "\n"
        R"(P-1\DLN:pcm1;)"
        "\n";
    TMATSParser parser(tmats);
    pcmdata_map output;
    TMATSData td;
    ASSERT_TRUE(td.ParsePCMAttributes(parser, output));
    ASSERT_EQ(2, output.size());
    ASSERT_EQ(1, output.count(1));
    ASSERT_EQ(1, output.count(4));

    Ch10PCMTMATSData exp1;
    exp1.common_word_length_ = 16;
    exp1.words_in_min_frame_ = 101;
    exp1.bits_in_min_frame_ = 201;
    exp1.min_frames_in_maj_frame_ = 31;
    exp1.type_format_ = "tf";
    exp1.polarity_ = "N";
    exp1.data_link_name_ = "pcm1";
    EXPECT_EQ(exp1, output.at(1));

    Ch10PCMTMATSData exp2;
    exp2.common_word_length_ = 20;
    exp2.words_in_min_frame_ = 104;
    exp2.bits_in_min_frame_ = 204;
    exp2.min_frames_in_maj_frame_ = 34;
    exp2.type_format_ = "tf4";
    exp2.polarity_ = "null";
    exp2.data_link_name_ = "pcm4";
    EXPECT_EQ(exp2, output.at(4));
}