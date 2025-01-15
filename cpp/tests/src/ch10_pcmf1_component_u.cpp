#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_pcmf1_component.h"

class Ch10PCMF1ComponentTest : public ::testing::Test
{
   protected:
    Ch10PCMF1Component comp_;
    const uint8_t* data_ptr_;
    Ch10Status status_;
    Ch10Context ctx_;

    Ch10PCMF1ComponentTest() : data_ptr_(nullptr), status_(Ch10Status::NONE), 
        ctx_(0), comp_(&ctx_) 
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
    EXPECT_EQ(16, comp_.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));
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
    EXPECT_EQ(32, comp_.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));
}

TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameSyncPatternBitCountAlign16UnpackedGT32)
{
    int pattern_len = 45;
    PCMF1CSDWFmt hdr{};
    hdr.mode_align = 0;
    hdr.mode_packed = 0;
    hdr.mode_unpacked = 1;
    hdr.mode_throughput = 0;
    int expected_word_count = (pattern_len + 15)/16;
    EXPECT_EQ(expected_word_count*16, 
        comp_.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    hdr.mode_align = 1;
    expected_word_count = (pattern_len + 31)/32;
    EXPECT_EQ(expected_word_count*32, 
        comp_.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));
}

TEST_F(Ch10PCMF1ComponentTest, GetPacketMinFrameSyncPatternBitCountPacked)
{
    int pattern_len = 15;
    PCMF1CSDWFmt hdr{};
    hdr.mode_align = 0;
    hdr.mode_packed = 1;
    hdr.mode_unpacked = 0;
    hdr.mode_throughput = 0;
    EXPECT_EQ(pattern_len, comp_.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    pattern_len = 25;
    EXPECT_EQ(pattern_len, comp_.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    hdr.mode_align = 1;
    EXPECT_EQ(pattern_len, comp_.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    pattern_len = 7;    
    EXPECT_EQ(pattern_len, comp_.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));

    pattern_len = 79;    
    EXPECT_EQ(pattern_len, comp_.GetPacketMinFrameSyncPatternBitCount(&hdr, pattern_len));
}

TEST_F(Ch10PCMF1ComponentTest, CalculateMinorFrameCountNoThroughput)
{
    uint32_t pkt_data_sz = 12456;
    PCMF1CSDWFmt hdr{};
    hdr.mode_throughput = 1;  // throuhgput mode not allowed

    Ch10PCMTMATSData tmats;
    uint32_t minor_frame_count = 0;
    uint32_t minor_frame_size = 0;
    EXPECT_FALSE(comp_.CalculateMinorFrameCount(pkt_data_sz, tmats, 
        &hdr, minor_frame_count, minor_frame_size));
}

// MFSPlt16 = minor frame sync pattern less than 16 bits in length
// TEST_F(Ch10PCMF1ComponentTest, CalculateMinorFrameCount16UnpackedNoIPHMFSPlt16)
// {
//     uint32_t pkt_data_sz = 12456;
//     PCMF1CSDWFmt hdr{};
//     hdr.mode_packed = 0;
//     hdr.mode_unpacked = 1;
//     hdr.mode_throughput = 0;
//     hdr.IPH = 0;
//     hdr.mode_align = 16;

//     Ch10PCMTMATSData tmats;
//     tmats.bits_in_min_frame_ = 377;
//     tmats.min_frame_sync_pattern_len_ = 15;

//     uint32_t minor_frame_count = 0;
//     uint32_t minor_frame_size = 0;
//     EXPECT_FALSE(comp_.CalculateMinorFrameCount(pkt_data_sz, tmats, 
//         &hdr, minor_frame_count, minor_frame_size));
// }
