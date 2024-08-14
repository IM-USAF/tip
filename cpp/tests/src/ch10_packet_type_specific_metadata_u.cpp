#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parser_metadata.h"
#include "md_category_map.h"
#include "tmats_data.h"
#include "ch10_context_mock.h"
#include "tmats_data_mock.h"
#include "ch10_packet_type_specific_metadata_mock.h"

using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::_;
using ::testing::InSequence;
using ::testing::Sequence;

class Ch10PacketTypeSpecificMetadataTest : public ::testing::Test
{
    protected:
        Ch10PacketTypeSpecificMetadata pm_;
        Ch10PacketTypeSpecificMetadataFunctions fm_;
        bool result_;

        Ch10PacketTypeSpecificMetadataTest() : pm_(), fm_(), result_(false)
        {}
};

TEST_F(Ch10PacketTypeSpecificMetadataTest, RecordMilStd1553F1SpecificMetadataCombineChannelIDFail)
{
    std::map<uint32_t, std::set<uint16_t>> ctx1_map1{{12, {431, 881}}};
    std::map<uint32_t, std::set<uint16_t>> ctx1_map2{{14, {31, 81}}};
    std::map<uint32_t, std::set<uint16_t>> ctx2_map1{{2, {43, 88}}};
    std::map<uint32_t, std::set<uint16_t>> ctx2_map2{{1, {41, 81}}};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr1_maps{ctx1_map1, ctx2_map1};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr2_maps{ctx1_map2, ctx2_map2};

    MockCh10Context ctx1;
    MockCh10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    EXPECT_CALL(ctx1, GetChannelIDToRemoteAddr1Map()).WillOnce(Return(ctx1_map1));
    EXPECT_CALL(ctx1, GetChannelIDToRemoteAddr2Map()).WillOnce(Return(ctx1_map2));
    EXPECT_CALL(ctx2, GetChannelIDToRemoteAddr1Map()).WillOnce(Return(ctx2_map1));
    EXPECT_CALL(ctx2, GetChannelIDToRemoteAddr2Map()).WillOnce(Return(ctx2_map2));

    MockCh10PacketTypeSpecificMetadataFunctions func_mock;
    std::map<uint32_t, std::set<uint16_t>> output_remoteaddr_map{{321, {10, 12, 13}}};
    EXPECT_CALL(func_mock, CombineChannelIDToLRUAddressesMetadata(_,
        chanid_lruaddr1_maps, chanid_lruaddr2_maps)).WillOnce(
            DoAll(SetArgReferee<0>(output_remoteaddr_map), Return(false)));

    MDCategoryMap runtime("runtime");
    EXPECT_FALSE(pm_.RecordMilStd1553F1SpecificMetadata(ctx_vec, &runtime, &func_mock));
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, RecordMilStd1553F1SpecificMetadataCombineCommWordsFail)
{
    std::map<uint32_t, std::set<uint16_t>> ctx1_map1{{12, {431, 881}}};
    std::map<uint32_t, std::set<uint16_t>> ctx1_map2{{14, {31, 81}}};
    std::map<uint32_t, std::set<uint16_t>> ctx2_map1{{2, {43, 88}}};
    std::map<uint32_t, std::set<uint16_t>> ctx2_map2{{1, {41, 81}}};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr1_maps{ctx1_map1, ctx2_map1};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr2_maps{ctx1_map2, ctx2_map2};

    MockCh10Context ctx1;
    MockCh10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    EXPECT_CALL(ctx1, GetChannelIDToRemoteAddr1Map()).WillOnce(Return(ctx1_map1));
    EXPECT_CALL(ctx1, GetChannelIDToRemoteAddr2Map()).WillOnce(Return(ctx1_map2));
    EXPECT_CALL(ctx2, GetChannelIDToRemoteAddr1Map()).WillOnce(Return(ctx2_map1));
    EXPECT_CALL(ctx2, GetChannelIDToRemoteAddr2Map()).WillOnce(Return(ctx2_map2));

    MockCh10PacketTypeSpecificMetadataFunctions func_mock;
    std::map<uint32_t, std::set<uint16_t>> output_remoteaddr_map{{321, {10, 12, 13}}};
    EXPECT_CALL(func_mock, CombineChannelIDToLRUAddressesMetadata(_,
        chanid_lruaddr1_maps, chanid_lruaddr2_maps)).WillOnce(
            DoAll(SetArgReferee<0>(output_remoteaddr_map), Return(true)));

    std::map<uint32_t, std::set<uint32_t>> ctx1_commwords_map{{519, {4210, 521}}};
    std::map<uint32_t, std::set<uint32_t>> ctx2_commwords_map{{8886, {8480, 51}}};
    std::vector<std::map<uint32_t, std::set<uint32_t>>> chanid_commwords_maps{
        ctx1_commwords_map, ctx2_commwords_map};

    EXPECT_CALL(ctx1, GetChannelIDToCommWordsMap()).WillOnce(Return(ctx1_commwords_map));
    EXPECT_CALL(ctx2, GetChannelIDToCommWordsMap()).WillOnce(Return(ctx2_commwords_map));

    std::map<uint32_t, std::vector<std::vector<uint32_t>>> output_chanid_commwords_map{
        {4343, {{1, 2, 3}, {4, 5, 6}}}};
    EXPECT_CALL(func_mock, CombineChannelIDToCommandWordsMetadata(_, chanid_commwords_maps)).
        WillOnce(DoAll(SetArgReferee<0>(output_chanid_commwords_map), Return(false)));

    MDCategoryMap runtime("runtime");
    EXPECT_FALSE(pm_.RecordMilStd1553F1SpecificMetadata(ctx_vec, &runtime, &func_mock));
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, RecordMilStd1553F1SpecificMetadata)
{
    std::map<uint32_t, std::set<uint16_t>> ctx1_map1{{12, {431, 881}}};
    std::map<uint32_t, std::set<uint16_t>> ctx1_map2{{14, {31, 81}}};
    std::map<uint32_t, std::set<uint16_t>> ctx2_map1{{2, {43, 88}}};
    std::map<uint32_t, std::set<uint16_t>> ctx2_map2{{1, {41, 81}}};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr1_maps{ctx1_map1, ctx2_map1};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr2_maps{ctx1_map2, ctx2_map2};

    MockCh10Context ctx1;
    MockCh10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    EXPECT_CALL(ctx1, GetChannelIDToRemoteAddr1Map()).WillOnce(Return(ctx1_map1));
    EXPECT_CALL(ctx1, GetChannelIDToRemoteAddr2Map()).WillOnce(Return(ctx1_map2));
    EXPECT_CALL(ctx2, GetChannelIDToRemoteAddr1Map()).WillOnce(Return(ctx2_map1));
    EXPECT_CALL(ctx2, GetChannelIDToRemoteAddr2Map()).WillOnce(Return(ctx2_map2));

    MockCh10PacketTypeSpecificMetadataFunctions func_mock;
    std::map<uint32_t, std::set<uint16_t>> output_remoteaddr_map{{321, {10, 12, 13}}};
    EXPECT_CALL(func_mock, CombineChannelIDToLRUAddressesMetadata(_,
        chanid_lruaddr1_maps, chanid_lruaddr2_maps)).WillOnce(
            DoAll(SetArgReferee<0>(output_remoteaddr_map), Return(true)));

    std::map<uint32_t, std::set<uint32_t>> ctx1_commwords_map{{519, {4210, 521}}};
    std::map<uint32_t, std::set<uint32_t>> ctx2_commwords_map{{8886, {8480, 51}}};
    std::vector<std::map<uint32_t, std::set<uint32_t>>> chanid_commwords_maps{
        ctx1_commwords_map, ctx2_commwords_map};

    EXPECT_CALL(ctx1, GetChannelIDToCommWordsMap()).WillOnce(Return(ctx1_commwords_map));
    EXPECT_CALL(ctx2, GetChannelIDToCommWordsMap()).WillOnce(Return(ctx2_commwords_map));

    std::map<uint32_t, std::vector<std::vector<uint32_t>>> output_chanid_commwords_map{
        {4343, {{1, 2, 3}, {4, 5, 6}}}};
    EXPECT_CALL(func_mock, CombineChannelIDToCommandWordsMetadata(_, chanid_commwords_maps)).
        WillOnce(DoAll(SetArgReferee<0>(output_chanid_commwords_map), Return(true)));

    MDCategoryMap runtime("runtime");
    EXPECT_TRUE(pm_.RecordMilStd1553F1SpecificMetadata(ctx_vec, &runtime, &func_mock));

    YAML::Node chanid_lruaddrs_node = runtime.node["chanid_to_lru_addrs"];
    ASSERT_TRUE(chanid_lruaddrs_node.IsMap());
    YAML::Node chanid_lruaddrs_set_node = chanid_lruaddrs_node["321"];
    ASSERT_TRUE(chanid_lruaddrs_set_node.IsSequence());
    EXPECT_THAT(chanid_lruaddrs_set_node.as<std::vector<uint16_t>>(),
        ::testing::UnorderedElementsAre(10, 12, 13));

    YAML::Node chanid_commwords_node = runtime.node["chanid_to_comm_words"];
    ASSERT_TRUE(chanid_commwords_node.IsMap());
    YAML::Node chanid_commwords_vec_node = chanid_commwords_node["4343"];
    ASSERT_TRUE(chanid_commwords_vec_node.IsSequence());
    ASSERT_EQ(2, chanid_commwords_vec_node.size());
    YAML::Node chanid_commwords_seq1 = chanid_commwords_vec_node[0];
    ASSERT_TRUE(chanid_commwords_seq1.IsSequence());
    YAML::Node chanid_commwords_seq2 = chanid_commwords_vec_node[1];
    ASSERT_TRUE(chanid_commwords_seq2.IsSequence());

    std::vector<uint32_t> seq1_expected = output_chanid_commwords_map.at(4343).at(0);
    std::vector<uint32_t> seq1 = chanid_commwords_seq1.as<std::vector<uint32_t>>();
    EXPECT_THAT(seq1_expected, ::testing::ElementsAreArray(seq1));

    std::vector<uint32_t> seq2_expected = output_chanid_commwords_map.at(4343).at(1);
    std::vector<uint32_t> seq2 = chanid_commwords_seq2.as<std::vector<uint32_t>>();
    EXPECT_THAT(seq2_expected, ::testing::ElementsAreArray(seq2));
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, RecordVideoDataF0SpecificMetadata)
{
    std::map<uint16_t, uint64_t> ctx1_map{{12, 881}};
    std::map<uint16_t, uint64_t> ctx2_map{{142, 431881}};
    std::vector<std::map<uint16_t, uint64_t>> worker_chanid_to_mintimestamps_maps{
        ctx1_map, ctx2_map};

    MockCh10Context ctx1;
    MockCh10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    EXPECT_CALL(ctx1, GetChannelIDToMinVideoTimestampMap()).WillOnce(Return(ctx1_map));
    EXPECT_CALL(ctx2, GetChannelIDToMinVideoTimestampMap()).WillOnce(Return(ctx2_map));

    MockCh10PacketTypeSpecificMetadataFunctions func_mock;

    std::map<uint16_t, uint64_t> output_min_timestamp_map{{34, 1982394939}, {35, 198384050}};
    EXPECT_CALL(func_mock, CreateChannelIDToMinVideoTimestampsMetadata(_,
        worker_chanid_to_mintimestamps_maps)).WillOnce(
            SetArgReferee<0>(output_min_timestamp_map));

    MDCategoryMap runtime("runtime");
    EXPECT_TRUE(pm_.RecordVideoDataF0SpecificMetadata(ctx_vec, &runtime, &func_mock));

    YAML::Node min_ts_node = runtime.node["chanid_to_first_timestamp"];
    ASSERT_TRUE(min_ts_node.IsMap());
    ASSERT_EQ(2, min_ts_node.size());
    EXPECT_THAT(output_min_timestamp_map, ::testing::ContainerEq(
        min_ts_node.as<std::map<uint16_t, uint64_t>>()));
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, RecordARINC429F0SpecificMetadataCombineChanIDLabelsFail)
{
    std::map<uint32_t, std::set<uint16_t>> chanid_labels_map1{{4456, {643, 12}}};
    std::map<uint32_t, std::set<uint16_t>> chanid_labels_map2{{456, {43, 2}}};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_label_maps{
        chanid_labels_map1, chanid_labels_map2};

    MockCh10Context ctx1;
    MockCh10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};
    MockCh10PacketTypeSpecificMetadataFunctions func_mock;

    EXPECT_CALL(ctx1, GetChannelIDToLabelsMap()).WillOnce(Return(chanid_labels_map1));
    EXPECT_CALL(ctx2, GetChannelIDToLabelsMap()).WillOnce(Return(chanid_labels_map2));

    std::map<uint32_t, std::set<uint16_t>> output_chanid_label_map{{4119, {324, 654, 881}}};
    EXPECT_CALL(func_mock, CombineChannelIDToLabelsMetadata(_, chanid_label_maps)).
        WillOnce(DoAll(SetArgReferee<0>(output_chanid_label_map), Return(false)));

    MDCategoryMap runtime("runtime");
    MockTMATSData tmats_mock;
    EXPECT_FALSE(pm_.RecordARINC429F0SpecificMetadata(ctx_vec, &runtime, &tmats_mock, &func_mock));

}

TEST_F(Ch10PacketTypeSpecificMetadataTest, RecordARINC429F0SpecificMetadataCombineChanIDBusNumbersFail)
{
    std::map<uint32_t, std::set<uint16_t>> chanid_labels_map1{{4456, {643, 12}}};
    std::map<uint32_t, std::set<uint16_t>> chanid_labels_map2{{456, {43, 2}}};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_label_maps{
        chanid_labels_map1, chanid_labels_map2};

    MockCh10Context ctx1;
    MockCh10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};
    MockCh10PacketTypeSpecificMetadataFunctions func_mock;

    EXPECT_CALL(ctx1, GetChannelIDToLabelsMap()).WillOnce(Return(chanid_labels_map1));
    EXPECT_CALL(ctx2, GetChannelIDToLabelsMap()).WillOnce(Return(chanid_labels_map2));

    std::map<uint32_t, std::set<uint16_t>> output_chanid_label_map{{4119, {324, 654, 881}}};
    EXPECT_CALL(func_mock, CombineChannelIDToLabelsMetadata(_, chanid_label_maps)).
        WillOnce(DoAll(SetArgReferee<0>(output_chanid_label_map), Return(true)));

    MDCategoryMap runtime("runtime");
    MockTMATSData tmats_mock;

    std::map<uint32_t, std::set<uint16_t>> busnum_map1{{104, {549, 828}}};
    std::map<uint32_t, std::set<uint16_t>> busnum_map2{{14, {54, 82}}};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_busnumber_maps{
        busnum_map1, busnum_map2};

    EXPECT_CALL(ctx1, GetChannelIDToBusNumbersMap()).WillOnce(Return(busnum_map1));
    EXPECT_CALL(ctx2, GetChannelIDToBusNumbersMap()).WillOnce(Return(busnum_map2));

    std::map<uint32_t, std::set<uint16_t>> output_chanid_busnumber_map{{5698, {54, 55, 56}}};
    EXPECT_CALL(func_mock, CombineChannelIDToBusNumbersMetadata(_, chanid_busnumber_maps)).
        WillOnce(DoAll(SetArgReferee<0>(output_chanid_busnumber_map), Return(false)));

    EXPECT_FALSE(pm_.RecordARINC429F0SpecificMetadata(ctx_vec, &runtime, &tmats_mock, &func_mock));
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, RecordARINC429F0SpecificMetadata)
{
    std::map<uint32_t, std::set<uint16_t>> chanid_labels_map1{{4456, {643, 12}}};
    std::map<uint32_t, std::set<uint16_t>> chanid_labels_map2{{456, {43, 2}}};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_label_maps{
        chanid_labels_map1, chanid_labels_map2};

    MockCh10Context ctx1;
    MockCh10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};
    MockCh10PacketTypeSpecificMetadataFunctions func_mock;

    EXPECT_CALL(ctx1, GetChannelIDToLabelsMap()).WillOnce(Return(chanid_labels_map1));
    EXPECT_CALL(ctx2, GetChannelIDToLabelsMap()).WillOnce(Return(chanid_labels_map2));

    std::map<uint32_t, std::set<uint16_t>> output_chanid_label_map{{4119, {324, 654, 881}}};
    EXPECT_CALL(func_mock, CombineChannelIDToLabelsMetadata(_, chanid_label_maps)).
        WillOnce(DoAll(SetArgReferee<0>(output_chanid_label_map), Return(true)));

    MDCategoryMap runtime("runtime");
    MockTMATSData tmats_mock;

    std::map<uint32_t, std::set<uint16_t>> busnum_map1{{104, {549, 828}}};
    std::map<uint32_t, std::set<uint16_t>> busnum_map2{{14, {54, 82}}};
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_busnumber_maps{
        busnum_map1, busnum_map2};

    EXPECT_CALL(ctx1, GetChannelIDToBusNumbersMap()).WillOnce(Return(busnum_map1));
    EXPECT_CALL(ctx2, GetChannelIDToBusNumbersMap()).WillOnce(Return(busnum_map2));

    std::map<uint32_t, std::set<uint16_t>> output_chanid_busnumber_map{{5698, {54, 55, 56}}};
    EXPECT_CALL(func_mock, CombineChannelIDToBusNumbersMetadata(_, chanid_busnumber_maps)).
        WillOnce(DoAll(SetArgReferee<0>(output_chanid_busnumber_map), Return(true)));

    cmap chanid_to_429_format{{"format1", "fmtval1"}};
    EXPECT_CALL(tmats_mock, GetChannelIDTo429Format()).WillOnce(ReturnRef(chanid_to_429_format));

    cmapvec chanid_to_429_subchan{{"subchan", {"alpha", "beta", "gamma"}}};
    EXPECT_CALL(tmats_mock, GetChannelIDTo429Subchans()).WillOnce(ReturnRef(chanid_to_429_subchan));

    cmapmap chanid_to_429_subchanname{{"subchanname", {{"key1", "val1"}}}};
    EXPECT_CALL(tmats_mock, GetChannelIDTo429SubchanAndName()).WillOnce(
        ReturnRef(chanid_to_429_subchanname));

    EXPECT_TRUE(pm_.RecordARINC429F0SpecificMetadata(ctx_vec, &runtime, &tmats_mock, &func_mock));

    YAML::Node chanid_labels_node = runtime.node["chanid_to_labels"];
    YAML::Node chanid_busnum_node = runtime.node["chanid_to_bus_numbers"];
    YAML::Node chanid_429format_node = runtime.node["tmats_chanid_to_429_format"];
    YAML::Node chanid_429subchan_node = runtime.node["tmats_chanid_to_429_subchans"];
    YAML::Node chanid_429subchanname_node = runtime.node["tmats_chanid_to_429_subchan_and_name"];

    ASSERT_TRUE(chanid_labels_node.IsMap());
    ASSERT_TRUE(chanid_busnum_node.IsMap());
    ASSERT_TRUE(chanid_429format_node.IsMap());
    ASSERT_TRUE(chanid_429subchan_node.IsMap());
    ASSERT_TRUE(chanid_429subchanname_node.IsMap());

    ASSERT_EQ(3, chanid_labels_node["4119"].size());
    YAML::Node chanid_labels_seq_node = chanid_labels_node["4119"];
    ASSERT_TRUE(chanid_labels_seq_node.IsSequence());
    EXPECT_EQ("654", chanid_labels_seq_node[1].Scalar());

    ASSERT_EQ(3, chanid_busnum_node["5698"].size());
    EXPECT_EQ("56", chanid_busnum_node["5698"][2].Scalar());

    ASSERT_EQ(1, chanid_429format_node.size());
    EXPECT_EQ(chanid_to_429_format.at("format1"), chanid_429format_node["format1"].Scalar());

    ASSERT_EQ(1, chanid_429subchan_node.size());
    ASSERT_EQ(3, chanid_429subchan_node["subchan"].size());
    EXPECT_EQ("gamma", chanid_429subchan_node["subchan"][2].Scalar());

    ASSERT_EQ(1, chanid_429subchanname_node.size());
    ASSERT_EQ(1, chanid_429subchanname_node["subchanname"].size());
    EXPECT_EQ("val1", chanid_429subchanname_node["subchanname"]["key1"].Scalar());
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, CombineChannelIDToLRUAddressesMetadataUnequalLengthVectors)
{
    std::map<uint32_t, std::set<uint16_t>> output;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr1_maps;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr2_maps;
    std::map<uint32_t, std::set<uint16_t>> map1_0;
    chanid_lruaddr1_maps.push_back(map1_0);

    result_ = fm_.CombineChannelIDToLRUAddressesMetadata(output, chanid_lruaddr1_maps,
                                                        chanid_lruaddr2_maps);
    EXPECT_FALSE(result_);
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, CombineChannelIDToLRUAddressesMetadata)
{
    std::map<uint32_t, std::set<uint16_t>> output;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr1_maps;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr2_maps;
    std::map<uint32_t, std::set<uint16_t>> map1_0 = {
        {5, {10, 12}},
        {10, {11, 13}}};
    std::map<uint32_t, std::set<uint16_t>> map1_1 = {
        {5, {10, 15}},
    };
    std::map<uint32_t, std::set<uint16_t>> map2_0 = {
        {6, {10, 12}},
        {7, {9, 10}}};
    std::map<uint32_t, std::set<uint16_t>> map2_1 = {
        {8, {1, 2}},
        {9, {3, 4}}};

    chanid_lruaddr1_maps.push_back(map1_0);
    chanid_lruaddr1_maps.push_back(map1_1);

    chanid_lruaddr2_maps.push_back(map2_0);
    chanid_lruaddr2_maps.push_back(map2_1);

    std::map<uint32_t, std::set<uint16_t>> expected = {
        {5, {10, 12, 15}},
        {6, {10, 12}},
        {7, {9, 10}},
        {8, {1, 2}},
        {9, {3, 4}},
        {10, {11, 13}}};

    result_ = fm_.CombineChannelIDToLRUAddressesMetadata(output, chanid_lruaddr1_maps,
                                                        chanid_lruaddr2_maps);
    EXPECT_TRUE(result_);
    EXPECT_EQ(expected, output);
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, CombineChannelIDToCommandWordsMetadata)
{
    std::vector<std::map<uint32_t, std::set<uint32_t>>> chanid_commwords_maps;
    std::map<uint32_t, std::vector<std::vector<uint32_t>>> orig_commwords1 = {
        {16, {{10, 4211}, {752, 1511}}},
        {17, {{9, 23}}}};
    std::map<uint32_t, std::vector<std::vector<uint32_t>>> orig_commwords2 = {
        {16, {{10, 4211}, {1992, 1066}}},
        {18, {{900, 211}, {11, 9341}, {41, 55}}}};

    std::map<uint32_t, std::vector<std::vector<uint32_t>>>::const_iterator it;
    std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map1;
    for (it = orig_commwords1.cbegin(); it != orig_commwords1.cend(); ++it)
    {
        std::set<uint32_t> temp_set;
        for (std::vector<std::vector<uint32_t>>::const_iterator it2 = it->second.cbegin();
             it2 != it->second.cend(); ++it2)
        {
            temp_set.insert((it2->at(0) << 16) + it2->at(1));
        }
        chanid_commwords_map1[it->first] = temp_set;
    }
    chanid_commwords_maps.push_back(chanid_commwords_map1);

    std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map2;
    for (it = orig_commwords2.cbegin(); it != orig_commwords2.cend(); ++it)
    {
        std::set<uint32_t> temp_set;
        for (std::vector<std::vector<uint32_t>>::const_iterator it2 = it->second.cbegin();
             it2 != it->second.cend(); ++it2)
        {
            temp_set.insert((it2->at(0) << 16) + it2->at(1));
        }
        chanid_commwords_map2[it->first] = temp_set;
    }
    chanid_commwords_maps.push_back(chanid_commwords_map2);

    std::map<uint32_t, std::vector<std::vector<uint32_t>>> output_map;
    result_ = fm_.CombineChannelIDToCommandWordsMetadata(output_map, chanid_commwords_maps);
    EXPECT_TRUE(result_);
    EXPECT_EQ(3, output_map.size());
    EXPECT_THAT(output_map.at(16),
                ::testing::Contains(::testing::ElementsAreArray({10, 4211})));
    EXPECT_THAT(output_map.at(16),
                ::testing::Contains(::testing::ElementsAreArray({752, 1511})));
    EXPECT_THAT(output_map.at(16),
                ::testing::Contains(::testing::ElementsAreArray({1992, 1066})));

    EXPECT_THAT(output_map.at(17),
                ::testing::Contains(::testing::ElementsAreArray({9, 23})));

    EXPECT_THAT(output_map.at(18),
                ::testing::Contains(::testing::ElementsAreArray({900, 211})));
    EXPECT_THAT(output_map.at(18),
                ::testing::Contains(::testing::ElementsAreArray({11, 9341})));
    EXPECT_THAT(output_map.at(18),
                ::testing::Contains(::testing::ElementsAreArray({41, 55})));
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, CreateChannelIDToMinVideoTimestampsMetadata)
{
    std::map<uint16_t, uint64_t> output_map;
    std::vector<std::map<uint16_t, uint64_t>> input_maps = {
        {{12, 100}, {13, 120}, {14, 98}},
        {{12, 100}, {13, 110}, {14, 200}},
        {{12, 120}, {13, 108}, {14, 150}}};
    std::map<uint16_t, uint64_t> expected = {
        {12, 100}, {13, 108}, {14, 98}};
    fm_.CreateChannelIDToMinVideoTimestampsMetadata(output_map,
                                                   input_maps);
    EXPECT_EQ(expected, output_map);
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, CombineChannelIDToLabelsMetadata)
{
    std::map<uint32_t, std::set<uint16_t>> output;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_labels_maps;

    std::map<uint32_t, std::set<uint16_t>> map1_0 = {
        {5, {194, 202}},
        {10, {193, 202}}};
    std::map<uint32_t, std::set<uint16_t>> map1_1 = {
        {5, {201}},
        {10, {67}}};

    chanid_labels_maps.push_back(map1_0);
    chanid_labels_maps.push_back(map1_1);

    std::map<uint32_t, std::set<uint16_t>> expected = {
        {5, {194, 201, 202}},
        {10, {67, 193, 202}}};

    result_ = fm_.CombineChannelIDToLabelsMetadata(output,
                                                  chanid_labels_maps);
    EXPECT_TRUE(result_);
    EXPECT_EQ(expected, output);
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, CombineChannelIDToBusNumbersMetadata)
{
    std::map<uint32_t, std::set<uint16_t>> output;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_busnumbers_maps;

    std::map<uint32_t, std::set<uint16_t>> map1_0 = {
        {5, {1, 2}},
        {10, {1, 2, 3}}};
    std::map<uint32_t, std::set<uint16_t>> map1_1 = {
        {5, {5}},
        {10, {3, 4}}};

    chanid_busnumbers_maps.push_back(map1_0);
    chanid_busnumbers_maps.push_back(map1_1);

    std::map<uint32_t, std::set<uint16_t>> expected = {
        {5, {1, 2, 5}},
        {10, {1, 2, 3, 4}}};

    result_ = fm_.CombineChannelIDToBusNumbersMetadata(output,
                                                  chanid_busnumbers_maps);
    EXPECT_TRUE(result_);
    EXPECT_EQ(expected, output);
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, CombineChannelIDToBusNumbersToLabelsMetadata)
{
    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> output;
    std::vector<std::map<uint32_t, std::map<uint32_t,
        std::set<uint16_t>>>> chanid_busnumbers_labels_maps;

    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> map1_0 = {
        {5, {{1, {194, 202}}, {2, {201}}}},
        {10, {{1, {193, 202}}}}
        };
    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> map1_1 = {
        {5, {{1, {201}}}},
        {10, {{1, {67}}}}
        };

    chanid_busnumbers_labels_maps.push_back(map1_0);
    chanid_busnumbers_labels_maps.push_back(map1_1);

    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> expected = {
        {5, {{1, {194, 201, 202}}, {2, {201}}}},
        {10, {{1, {67, 193, 202}}}}};

    result_ = fm_.CombineChannelIDToBusNumbersToLabelsMetadata(output,
                                                  chanid_busnumbers_labels_maps);
    EXPECT_TRUE(result_);
    EXPECT_EQ(expected, output);
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, PopulatePCMDataObject)
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

    Ch10PCMTMATSData pcmdata;
    ASSERT_TRUE(fm_.PopulatePCMDataObject(code_to_vals, pcmdata));
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

TEST_F(Ch10PacketTypeSpecificMetadataTest, PopulatePCMDataObjectNullHandling)
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

    Ch10PCMTMATSData pcmdata;
    ASSERT_TRUE(fm_.PopulatePCMDataObject(code_to_vals, pcmdata));
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

TEST_F(Ch10PacketTypeSpecificMetadataTest, PopulatePCMDataObjectCastErrorFloat)
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

    Ch10PCMTMATSData pcmdata;
    ASSERT_FALSE(fm_.PopulatePCMDataObject(code_to_vals, pcmdata));
}

TEST_F(Ch10PacketTypeSpecificMetadataTest, PopulatePCMDataObjectCastErrorInt)
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

    Ch10PCMTMATSData pcmdata;
    ASSERT_FALSE(fm_.PopulatePCMDataObject(code_to_vals, pcmdata));
}