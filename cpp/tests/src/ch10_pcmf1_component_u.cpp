#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_context_mock.h"
#include "ch10_time_mock.h"
#include "ch10_pcmf1_component.h"
#include "ch10_pcmf1_component_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
// using ::testing::Property;
// using ::testing::Eq;
using ::testing::_;
// using ::testing::Ref;
using ::testing::DoAll;
using ::testing::SetArgReferee;

class Ch10PCMF1ComponentTest : public ::testing::Test
{
   protected:
    Ch10PCMF1Component comp_;
    const uint8_t* data_ptr_;
    Ch10Status status_;
    NiceMock<MockCh10PCMF1Component> mock_pcmf1_;
    Ch10Context ctx_;

    Ch10PCMF1ComponentTest() : data_ptr_(nullptr), status_(Ch10Status::NONE), 
        ctx_(0), comp_(&ctx_), mock_pcmf1_(&ctx_) 
    {}
};


TEST_F(Ch10PCMF1ComponentTest, CheckFrameIndicatorTrue)
{
    uint32_t throughput = 0;
    uint32_t MI = 1;
    uint32_t MA = 0;
    EXPECT_TRUE(comp_.CheckFrameIndicator(throughput, MI, MA));

    MI = 0;
    MA = 1;
    EXPECT_TRUE(comp_.CheckFrameIndicator(throughput, MI, MA));

    throughput = 1;
    EXPECT_TRUE(comp_.CheckFrameIndicator(throughput, MI, MA));

    MI = 1;
    EXPECT_TRUE(comp_.CheckFrameIndicator(throughput, MI, MA));

    MA = 0;
    EXPECT_TRUE(comp_.CheckFrameIndicator(throughput, MI, MA));
} 

TEST_F(Ch10PCMF1ComponentTest, CheckFrameIndicatorFalse)
{
    uint32_t throughput = 0;
    uint32_t MI = 1;
    uint32_t MA = 1;
    EXPECT_FALSE(comp_.CheckFrameIndicator(throughput, MI, MA));
}

// LT17 = less than 17
TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameSyncPatternBitCountUnpackedLT17)
{
    int pattern_len = 15;
    PCMF1CSDWFmt hdr{};
    hdr.mode_align = 0;
    hdr.mode_packed = 0;
    hdr.mode_unpacked = 1;
    hdr.mode_throughput = 0;
    Ch10PCMF1MinorFrame minframe;
    EXPECT_EQ(16, minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));
}

// GT16 = greater than 16
TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameSyncPatternBitCountUnpackedGT16)
{
    int pattern_len = 20;
    PCMF1CSDWFmt hdr{};
    hdr.mode_align = 0;
    hdr.mode_packed = 0;
    hdr.mode_unpacked = 1;
    hdr.mode_throughput = 0;
    Ch10PCMF1MinorFrame minframe;

    EXPECT_EQ(32, minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));
}

TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameSyncPatternBitCountAlign16UnpackedGT32)
{
    int pattern_len = 45;
    PCMF1CSDWFmt hdr{};
    hdr.mode_align = 0;
    hdr.mode_packed = 0;
    hdr.mode_unpacked = 1;
    hdr.mode_throughput = 0;
    Ch10PCMF1MinorFrame minframe;

    int expected_word_count = (pattern_len + 15)/16;
    EXPECT_EQ(expected_word_count*16, 
        minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    hdr.mode_align = 1;
    expected_word_count = (pattern_len + 31)/32;
    EXPECT_EQ(expected_word_count*32, 
        minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));
}

TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameSyncPatternBitCountPacked)
{
    int pattern_len = 15;
    PCMF1CSDWFmt hdr{};
    hdr.mode_align = 0;
    hdr.mode_packed = 1;
    hdr.mode_unpacked = 0;
    hdr.mode_throughput = 0;
    Ch10PCMF1MinorFrame minframe;

    EXPECT_EQ(pattern_len, minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    pattern_len = 25;
    EXPECT_EQ(pattern_len, minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    hdr.mode_align = 1;
    EXPECT_EQ(pattern_len, minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    pattern_len = 7;    
    EXPECT_EQ(pattern_len, minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    pattern_len = 79;    
    EXPECT_EQ(pattern_len, minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));
}

TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameSyncPatternBitCountThroughput)
{
    int pattern_len = 15;
    PCMF1CSDWFmt hdr{};
    hdr.mode_align = 0;
    hdr.mode_packed = 0;
    hdr.mode_unpacked = 0;
    hdr.mode_throughput = 1;
    Ch10PCMF1MinorFrame minframe;
    EXPECT_EQ(pattern_len, minframe.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));
}

// Note: May need to test various common word lengths. Not sure how to 
// handle if common word length is, for example, 24 bits. How to pad, etc?
// Need additional test for situation in which different word sizes are used.
TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameBitCountAlign16Unpacked)
{
    // This test only handles the case of common word length < 17 bits
    // with no non-common words lengths.
    Ch10PCMTMATSData tmats{};
    tmats.common_word_length_ = 16;
    tmats.words_in_min_frame_ = 10;
    PCMF1CSDWFmt hdr{};
    hdr.mode_unpacked = 1;
    hdr.mode_packed = 0;
    hdr.mode_throughput = 0;
    hdr.mode_align = 0;  // 16-bit
    int pkt_sync_pattern_bits = 16;
    Ch10PCMF1MinorFrame minframe;

    // Subtract the sync pattern word, which counts as one word regardless
    // of length
    int expected = (tmats.words_in_min_frame_ - 1)*16
        + pkt_sync_pattern_bits;      
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));

    pkt_sync_pattern_bits = 32;
    expected = (tmats.words_in_min_frame_ - 1)*16
        + pkt_sync_pattern_bits;      
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr,
        pkt_sync_pattern_bits));

}

TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameBitCountAlign32Unpacked)
{
    // This test only handles the case of common word length < 17 bits
    // with no non-common words lengths.
    Ch10PCMTMATSData tmats{};
    tmats.common_word_length_ = 16;
    tmats.words_in_min_frame_ = 10;
    PCMF1CSDWFmt hdr{};
    hdr.mode_unpacked = 1;
    hdr.mode_packed = 0;
    hdr.mode_throughput = 0;
    hdr.mode_align = 1;  // 32-bit
    int pkt_sync_pattern_bits = 16;
    Ch10PCMF1MinorFrame minframe;

    int expected = (tmats.words_in_min_frame_ - 1)*16
        + pkt_sync_pattern_bits;      
    if(expected % 32 != 0)
        expected += 16;
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));

    pkt_sync_pattern_bits = 32;
    expected = (tmats.words_in_min_frame_ - 1)*16
        + pkt_sync_pattern_bits;      
    if(expected % 32 != 0)
        expected += 16;
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));

    tmats.words_in_min_frame_ = 11;
    pkt_sync_pattern_bits = 16;
    expected = (tmats.words_in_min_frame_ - 1)*16
        + pkt_sync_pattern_bits;      
    if(expected % 32 != 0)
        expected += 16;
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));

    tmats.words_in_min_frame_ = 11;
    pkt_sync_pattern_bits = 32;
    expected = (tmats.words_in_min_frame_ - 1)*16
        + pkt_sync_pattern_bits;      
    if(expected % 32 != 0)
        expected += 16;
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));
}

TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameBitCountAlign16Packed)
{
    // This test does not handle the case of words which do not have
    // the common word length.
    Ch10PCMTMATSData tmats{};
    tmats.common_word_length_ = 14;
    tmats.words_in_min_frame_ = 10;
    tmats.min_frame_sync_pattern_len_ = 16;
    PCMF1CSDWFmt hdr{};
    hdr.mode_unpacked = 0;
    hdr.mode_packed = 1;
    hdr.mode_throughput = 0;
    hdr.mode_align = 0;  // 16-bit
    int pkt_sync_pattern_bits = 0;
    Ch10PCMF1MinorFrame minframe;

    int expected = (tmats.words_in_min_frame_ - 1)*tmats.common_word_length_
        + tmats.min_frame_sync_pattern_len_;      
    if(expected % 16 != 0)
    {
        int pad_bits = 16 - (expected % 16);
        expected += pad_bits;
    }
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));

    tmats.min_frame_sync_pattern_len_ = 18;
    expected = (tmats.words_in_min_frame_ - 1)*tmats.common_word_length_
        + tmats.min_frame_sync_pattern_len_;      
    if(expected % 16 != 0)
    {
        int pad_bits = 16 - (expected % 16);
        expected += pad_bits;
    }
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));
}

TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameBitCountThroughput)
{
    Ch10PCMTMATSData tmats{};
    tmats.common_word_length_ = 14;
    tmats.words_in_min_frame_ = 10;
    tmats.bits_in_min_frame_ = 150;
    tmats.min_frame_sync_pattern_len_ = 24;
    PCMF1CSDWFmt hdr{};
    hdr.mode_unpacked = 0;
    hdr.mode_packed = 0;
    hdr.mode_throughput = 1;
    hdr.mode_align = 1;  // 32-bit
    int pkt_sync_pattern_bits = 38;  // not relevant for calculation
    Ch10PCMF1MinorFrame minframe;

    EXPECT_EQ(tmats.bits_in_min_frame_, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));

    
    hdr.mode_align = 0;
    EXPECT_EQ(tmats.bits_in_min_frame_, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));
}


TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameBitCountAlign32Packed)
{
    // This test does not handle the case of words which do not have
    // the common word length.
    Ch10PCMTMATSData tmats{};
    tmats.common_word_length_ = 14;
    tmats.words_in_min_frame_ = 10;
    tmats.min_frame_sync_pattern_len_ = 16;
    PCMF1CSDWFmt hdr{};
    hdr.mode_unpacked = 0;
    hdr.mode_packed = 1;
    hdr.mode_throughput = 0;
    hdr.mode_align = 1;  // 32-bit
    int pkt_sync_pattern_bits = 0;
    Ch10PCMF1MinorFrame minframe;

    // 142
    int expected = (tmats.words_in_min_frame_ - 1)*tmats.common_word_length_
        + tmats.min_frame_sync_pattern_len_;      
    // 14
    if(expected % 32 != 0)
    {
        int pad_bits = 32 - (expected % 32);
        expected += pad_bits;
    }
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));

    tmats.words_in_min_frame_ = 9;
    // 128
    expected = (tmats.words_in_min_frame_ - 1)*tmats.common_word_length_
        + tmats.min_frame_sync_pattern_len_;      
    if(expected % 32 != 0)
    {
        int pad_bits = 32 - (expected % 32);
        expected += pad_bits;
    }
    EXPECT_EQ(expected, minframe.GetPacketMinFrameBitCount(tmats, &hdr, 
        pkt_sync_pattern_bits));
}

TEST_F(Ch10PCMF1ComponentTest, CalculateMinorFrameCountNoThroughput)
{
    uint32_t pkt_data_sz = 12456;
    PCMF1CSDWFmt hdr{};
    hdr.mode_throughput = 1;  // throuhgput mode not allowed

    Ch10PCMTMATSData tmats;
    uint32_t minor_frame_count = 0;
    uint32_t minor_frame_size = 0;
    Ch10PCMF1MinorFrame minframe;
    Ch10PCMF1Calculations calcs;
    EXPECT_FALSE(calcs.CalculateMinorFrameCount(&minframe, pkt_data_sz, tmats, 
        &hdr, minor_frame_count, minor_frame_size));
}

TEST_F(Ch10PCMF1ComponentTest, CalculateMinorFrameCountNoIPH)
{
    uint32_t pkt_data_sz = 12456;
    PCMF1CSDWFmt hdr{};
    hdr.mode_packed = 0;
    hdr.mode_unpacked = 1;
    hdr.mode_throughput = 0;
    hdr.IPH = 0;

    Ch10PCMTMATSData tmats;
    // not relevant for mocked methods, but required as input
    int input_sync_pattern_len_bits = 24;  
    tmats.min_frame_sync_pattern_len_ = input_sync_pattern_len_bits;
    int output_sync_pattern_len_bits = 32;
    int output_min_frame_bit_count = 432;
    int input_min_frame_count = 12;
    uint32_t input_pkt_data_sz = (output_min_frame_bit_count/8) * input_min_frame_count;
    uint32_t temp_minor_frame_count = 0;
    uint32_t temp_minor_frame_size = 0;
    Ch10PCMF1Calculations calcs;
    Ch10PCMF1MinorFrame minframe;
    EXPECT_FALSE(calcs.CalculateMinorFrameCount(&minframe, input_pkt_data_sz, 
        tmats, &hdr, temp_minor_frame_count, temp_minor_frame_size));
}

TEST_F(Ch10PCMF1ComponentTest, CalculateMinorFrameCountNonIntegerFrameCount)
{
    uint32_t pkt_data_sz = 12456;
    PCMF1CSDWFmt hdr{};
    hdr.mode_packed = 0;
    hdr.mode_unpacked = 1;
    hdr.mode_throughput = 0;
    hdr.IPH = 1;
    hdr.mode_align = 0;

    Ch10PCMTMATSData tmats;
    // not relevant for mocked methods, but required as input
    int input_sync_pattern_len_bits = 24;  
    tmats.min_frame_sync_pattern_len_ = input_sync_pattern_len_bits;
    int output_sync_pattern_len_bits = 32;
    int output_min_frame_bit_count = 432;
    int input_min_frame_count = 12;
    uint32_t input_pkt_data_sz = (output_min_frame_bit_count/8 + 10) * 
        input_min_frame_count + 23; // 23 is not multiple of minor frame size
    uint32_t temp_minor_frame_count = 0;
    uint32_t temp_minor_frame_size = 0;
    MockCh10PCMF1MinorFrame mock_minframe;
    Ch10PCMF1Calculations calcs;

    EXPECT_CALL(mock_minframe, GetPacketMinFrameSyncPatternBitCount(&hdr, 
        input_sync_pattern_len_bits)).WillOnce(
            Return(output_sync_pattern_len_bits));
    EXPECT_CALL(mock_minframe, GetPacketMinFrameBitCount(tmats, &hdr, 
        output_sync_pattern_len_bits)).WillOnce(Return(output_min_frame_bit_count));

    EXPECT_FALSE(calcs.CalculateMinorFrameCount(&mock_minframe, input_pkt_data_sz, 
        tmats, &hdr, temp_minor_frame_count, temp_minor_frame_size));
}

TEST_F(Ch10PCMF1ComponentTest, CalculateMinorFrameCount16BitMode)
{
    uint32_t pkt_data_sz = 12456;
    PCMF1CSDWFmt hdr{};
    hdr.mode_packed = 1;
    hdr.mode_unpacked = 0;
    hdr.mode_throughput = 0;
    hdr.IPH = 1;
    hdr.mode_align = 0;  // 16-bit 

    Ch10PCMTMATSData tmats;
    // not relevant for mocked methods, but required as input
    int input_sync_pattern_len_bits = 24;  
    tmats.min_frame_sync_pattern_len_ = input_sync_pattern_len_bits;
    int output_sync_pattern_len_bits = 32;
    int output_min_frame_bit_count = 432;
    int input_min_frame_count = 12;
    uint32_t input_pkt_data_sz = (output_min_frame_bit_count/8 + 10) * 
        input_min_frame_count;
    uint32_t temp_minor_frame_count = 0;
    uint32_t temp_minor_frame_size = 0;
    MockCh10PCMF1MinorFrame mock_minframe;
    Ch10PCMF1Calculations calcs;

    EXPECT_CALL(mock_minframe, GetPacketMinFrameSyncPatternBitCount(&hdr, 
        input_sync_pattern_len_bits)).WillOnce(
            Return(output_sync_pattern_len_bits));
    EXPECT_CALL(mock_minframe, GetPacketMinFrameBitCount(tmats, &hdr, 
        output_sync_pattern_len_bits)).WillOnce(Return(output_min_frame_bit_count));

    EXPECT_TRUE(calcs.CalculateMinorFrameCount(&mock_minframe, input_pkt_data_sz, 
        tmats, &hdr, temp_minor_frame_count, temp_minor_frame_size));
    EXPECT_EQ(temp_minor_frame_count, input_min_frame_count);
    EXPECT_EQ(temp_minor_frame_size, output_min_frame_bit_count/8);
    EXPECT_EQ(calcs.GetPktSyncPatternLenBytes(), 
        output_sync_pattern_len_bits / 8);
    EXPECT_EQ(calcs.GetPktMinFrameLenBytes(), 
        output_min_frame_bit_count / 8);
}

TEST_F(Ch10PCMF1ComponentTest, CalculateMinorFrameCount32BitMode)
{
    uint32_t pkt_data_sz = 12456;
    PCMF1CSDWFmt hdr{};
    hdr.mode_packed = 1;
    hdr.mode_unpacked = 0;
    hdr.mode_throughput = 0;
    hdr.IPH = 1;
    hdr.mode_align = 1;  // 32-bit 

    Ch10PCMTMATSData tmats;
    // not relevant for mocked methods, but required as input
    int input_sync_pattern_len_bits = 24;  
    tmats.min_frame_sync_pattern_len_ = input_sync_pattern_len_bits;
    int output_sync_pattern_len_bits = 32;
    int output_min_frame_bit_count = 432;
    int input_min_frame_count = 12;
    uint32_t input_pkt_data_sz = (output_min_frame_bit_count/8 + 12) * 
        input_min_frame_count;
    uint32_t temp_minor_frame_count = 0;
    uint32_t temp_minor_frame_size = 0;
    MockCh10PCMF1MinorFrame mock_minframe;
    Ch10PCMF1Calculations calcs;

    EXPECT_CALL(mock_minframe, GetPacketMinFrameSyncPatternBitCount(&hdr, 
        input_sync_pattern_len_bits)).WillOnce(
            Return(output_sync_pattern_len_bits));
    EXPECT_CALL(mock_minframe, GetPacketMinFrameBitCount(tmats, &hdr, 
        output_sync_pattern_len_bits)).WillOnce(Return(output_min_frame_bit_count));
    EXPECT_TRUE(calcs.CalculateMinorFrameCount(&mock_minframe, input_pkt_data_sz, 
        tmats, &hdr, temp_minor_frame_count, temp_minor_frame_size));
    EXPECT_EQ(temp_minor_frame_count, input_min_frame_count);
    EXPECT_EQ(temp_minor_frame_size, output_min_frame_bit_count/8);
    EXPECT_EQ(calcs.GetPktSyncPatternLenBytes(),
        output_sync_pattern_len_bits / 8);
    EXPECT_EQ(calcs.GetPktMinFrameLenBytes(), 
        output_min_frame_bit_count / 8);
}

TEST_F(Ch10PCMF1ComponentTest, ParseFramesCalculateMinorFrameCountFail)
{
    PCMF1CSDWFmt hdr{};
    Ch10PCMTMATSData tmats;
    uint32_t temp_minor_frame_count = 0;
    uint32_t temp_minor_frame_size = 0;
    MockCh10Context mock_ctx;
    MockCh10PCMF1Calculations mock_calcs;
    Ch10Time ch10time;
    // arbitrary at this stage, just need to ensure that correct vars are passed
    // in the right places
    uint32_t packet_data_size = 456;
    uint32_t minor_frame_count = 77;
    uint32_t minor_frame_size_bytes = 298;

    EXPECT_CALL(mock_ctx, GetPacketDataSizeBytes()).WillOnce(
        ReturnRef(packet_data_size));
    EXPECT_CALL(mock_calcs, CalculateMinorFrameCount(_, packet_data_size, tmats, 
        &hdr, _, _)).WillOnce(Return(false));
    EXPECT_EQ(Ch10Status::PCMF1_ERROR, comp_.ParseFrames(&mock_calcs, data_ptr_, 
        tmats, &hdr, &mock_ctx, &ch10time));
}

TEST_F(Ch10PCMF1ComponentTest, ParseFramesCalculateAbsTimeFail)
{
    PCMF1CSDWFmt hdr{};
    Ch10PCMTMATSData tmats;
    uint32_t temp_minor_frame_count = 0;
    uint32_t temp_minor_frame_size = 0;
    MockCh10Context mock_ctx;
    MockCh10PCMF1Calculations mock_calcs;
    Ch10Time ch10time;

    // arbitrary at this stage, just need to ensure that correct vars are passed
    // in the right places
    uint32_t packet_data_size = 456;
    uint32_t expected_minor_frame_count = 7;
    uint32_t expected_minor_frame_size_bytes = 298;
    uint32_t minor_frame_count = 0;
    uint32_t minor_frame_size_bytes = 0;
    uint64_t expected_abs_time = 584882;
    uint64_t abs_time = 0;

    EXPECT_CALL(mock_ctx, GetPacketDataSizeBytes()).WillOnce(
        ReturnRef(packet_data_size));
    EXPECT_CALL(mock_calcs, CalculateMinorFrameCount(_, packet_data_size, tmats, 
        &hdr, _, _)).WillOnce(DoAll(
        SetArgReferee<4>(expected_minor_frame_count),
        SetArgReferee<5>(expected_minor_frame_size_bytes), Return(true)));
    EXPECT_CALL(mock_calcs, CalculateAbsTime(data_ptr_, &ch10time, &mock_ctx, 
        _)).WillOnce(DoAll(SetArgReferee<3>(expected_abs_time), 
        Return(Ch10Status::INVALID_INTRAPKT_TS_SRC)));
    EXPECT_EQ(Ch10Status::PCMF1_ERROR, comp_.ParseFrames(&mock_calcs, data_ptr_, 
        tmats, &hdr, &mock_ctx, &ch10time));
}

// TEST_F(Ch10PCMF1ComponentTest, ParseMinorFrame16Bit)
// {
//     PCMF1CSDWFmt hdr{};
//     Ch10PCMTMATSData tmats;
//     MockCh10Context mock_ctx;
//     Ch10Time ch10time;
//     Ch10PCMF1Calculations calcs;

//     uint32_t packet_data_size = 456;
//     uint32_t expected_minor_frame_count = 7;
//     uint32_t expected_minor_frame_size_bytes = 298;
//     uint32_t minor_frame_count = 0;
//     uint32_t minor_frame_size_bytes = 0;
//     uint64_t expected_abs_time = 584882;
//     uint64_t abs_time = 0;


//     PCMF1IPDH16Fmt ipdh;
//     ipdh.lockst = 0b1011;  // minor frame check, major frame lock


// }

TEST_F(Ch10PCMF1ComponentTest, CalculateAbsTimeParseIPTSError)
{
    // ///////////////////// not functional yet
    // to do:
    // test CalculateAbsTime, integrate into ParseFrames
    // add tests for parseframes
    // continue to impl parseframe
    Ch10PCMF1Calculations calcs;
    MockCh10Context mock_ctx;
    MockCh10Time mock_ch10time;
    uint64_t expected_abs_time_ns = 0;
    uint64_t abs_time_ns = 0;
    uint64_t expected_ipts_time = 584838282;
    uint64_t ipts_time = 0;
    uint8_t intrapkt_ts_src = 9;  // arb
    uint8_t time_fmt = 23;  // arbi
    Ch10Status expected_status = Ch10Status::INVALID_INTRAPKT_TS_SRC;

    EXPECT_CALL(mock_ctx, GetIntrapktTSSrc()).WillOnce(ReturnRef(intrapkt_ts_src));
    EXPECT_CALL(mock_ctx, GetTimeFormat()).WillOnce(ReturnRef(time_fmt));
    EXPECT_CALL(mock_ch10time, ParseIPTS(data_ptr_, ipts_time, intrapkt_ts_src,
        time_fmt)).WillOnce(DoAll(SetArgReferee<1>(expected_ipts_time), 
        Return(expected_status)));
    EXPECT_EQ(expected_status, calcs.CalculateAbsTime(data_ptr_, &mock_ch10time, 
        &mock_ctx, abs_time_ns));
}

TEST_F(Ch10PCMF1ComponentTest, CalculateAbsTime)
{
    Ch10PCMF1Calculations calcs;
    MockCh10Context mock_ctx;
    MockCh10Time mock_ch10time;
    uint64_t expected_abs_time_ns = 0;
    uint64_t abs_time_ns = 0;
    uint64_t expected_ipts_time = 584838282;
    uint64_t ipts_time = 0;
    uint8_t intrapkt_ts_src = 9;  // arb
    uint8_t time_fmt = 23;  // arbi
    Ch10Status expected_status = Ch10Status::OK;

    EXPECT_CALL(mock_ctx, GetIntrapktTSSrc()).WillOnce(ReturnRef(intrapkt_ts_src));
    EXPECT_CALL(mock_ctx, GetTimeFormat()).WillOnce(ReturnRef(time_fmt));
    EXPECT_CALL(mock_ch10time, ParseIPTS(data_ptr_, ipts_time, intrapkt_ts_src,
        time_fmt)).WillOnce(DoAll(SetArgReferee<1>(expected_ipts_time), 
        Return(expected_status)));
    EXPECT_CALL(mock_ctx, CalculateIPTSAbsTime(expected_ipts_time)).WillOnce(
        ReturnRef(expected_abs_time_ns));
    Ch10Status status = calcs.CalculateAbsTime(data_ptr_, &mock_ch10time, 
        &mock_ctx, abs_time_ns);
    EXPECT_EQ(expected_status, status);
    EXPECT_EQ(expected_abs_time_ns, abs_time_ns);
}

TEST_F(Ch10PCMF1ComponentTest, ParseSyncPattern16BitInsufficientVectorSizeUnpacked)
{
    PCMF1CSDWFmt hdr{};
    hdr.mode_unpacked = 1;
    Ch10PCMTMATSData tmats;
    uint32_t pkt_sync_pattern_len_bytes = 4;
    std::vector<uint16_t> parsed(1);  // should be (2) 16-bit units

    Ch10PCMF1MinorFrame minframe;
    EXPECT_FALSE(minframe.ParseSyncPattern16Bit(data_ptr_, 
        tmats, &hdr, pkt_sync_pattern_len_bytes, parsed)); 
}

TEST_F(Ch10PCMF1ComponentTest, ParseSyncPattern16BitInsufficientVectorSizePacked)
{
    PCMF1CSDWFmt hdr{};
    hdr.mode_packed = 1;
    Ch10PCMTMATSData tmats;
    uint32_t pkt_sync_pattern_len_bytes = 4;
    tmats.min_frame_sync_pattern_len_ = 35;      
    std::vector<uint16_t> parsed(1);  // should be (3) 16-bit units

    Ch10PCMF1MinorFrame minframe;
    EXPECT_FALSE(minframe.ParseSyncPattern16Bit(data_ptr_, 
        tmats, &hdr, pkt_sync_pattern_len_bytes, parsed)); 
}

TEST_F(Ch10PCMF1ComponentTest, ParseSyncPattern32BitInsufficientVectorSizeUnpacked)
{
    PCMF1CSDWFmt hdr{};
    hdr.mode_unpacked = 1;
    Ch10PCMTMATSData tmats;
    uint32_t pkt_sync_pattern_len_bytes = 9;
    std::vector<uint32_t> parsed(2);  // should be (3) 32-bit units

    Ch10PCMF1MinorFrame minframe;
    EXPECT_FALSE(minframe.ParseSyncPattern32Bit(data_ptr_, 
        tmats, &hdr, pkt_sync_pattern_len_bytes, parsed)); 
}

TEST_F(Ch10PCMF1ComponentTest, ParseSyncPattern32BitInsufficientVectorSizePacked)
{
    PCMF1CSDWFmt hdr{};
    hdr.mode_packed = 1;
    Ch10PCMTMATSData tmats;
    uint32_t pkt_sync_pattern_len_bytes = 4;
    tmats.min_frame_sync_pattern_len_ = 109;      
    std::vector<uint32_t> parsed(2);  // should be (4) 32-bit units

    Ch10PCMF1MinorFrame minframe;
    EXPECT_FALSE(minframe.ParseSyncPattern32Bit(data_ptr_, 
        tmats, &hdr, pkt_sync_pattern_len_bytes, parsed)); 
}