#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_pcm_tmats_data.h"

TEST(Ch10PCMTMATSDataTest, CalculateMajorFrameLengthTrue)
{
    Ch10PCMTMATSData d;
    d.bits_in_min_frame_ = 1604;
    d.words_in_min_frame_ = 100;
    d.min_frames_in_maj_frame_ = 20;
    d.common_word_length_ = 16;
    d.min_frame_sync_pattern_len_ = 20;

    int majframe_len = d.bits_in_min_frame_ * d.min_frames_in_maj_frame_;
    int majframe_len_bits = ((d.words_in_min_frame_ - 1)*d.common_word_length_
        + d.min_frame_sync_pattern_len_) * d.min_frames_in_maj_frame_;

    EXPECT_EQ(majframe_len, majframe_len_bits);
    int output = 0;
    EXPECT_TRUE(d.CalculateMajorFrameLength(output));
    EXPECT_EQ(output, majframe_len);
} 

TEST(Ch10PCMTMATSDataTest, CalculateMajorFrameLengthFalse)
{
    Ch10PCMTMATSData d;
    d.bits_in_min_frame_ = 1500;  // not 1604
    d.words_in_min_frame_ = 100;
    d.min_frames_in_maj_frame_ = 20;
    d.common_word_length_ = 16;
    d.min_frame_sync_pattern_len_ = 20;

    int majframe_len = d.bits_in_min_frame_ * d.min_frames_in_maj_frame_;
    int majframe_len_bits = ((d.words_in_min_frame_ - 1)*d.common_word_length_
        + d.min_frame_sync_pattern_len_) * d.min_frames_in_maj_frame_;

    int output = 0;
    EXPECT_FALSE(d.CalculateMajorFrameLength(output));
    EXPECT_EQ(output, -1);
}