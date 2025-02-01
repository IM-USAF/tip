#ifndef CH10_PCMF1_COMPONENT_MOCK_H_
#define CH10_PCMF1_COMPONENT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_pcmf1_component.h"

class MockCh10PCMF1Component: public Ch10PCMF1Component
{
   public:
    MockCh10PCMF1Component(Ch10Context* const ch10ctx) : Ch10PCMF1Component(ch10ctx) {}
    MOCK_METHOD1(Parse, Ch10Status(const uint8_t*& data));
};

class MockCh10PCMF1MinorFrame: public Ch10PCMF1MinorFrame
{
   public:
    MockCh10PCMF1MinorFrame() : Ch10PCMF1MinorFrame() {}
    MOCK_METHOD2(GetPacketMinFrameSyncPatternBitCount, int(
        const PCMF1CSDWFmt* hdr, const int& sync_pattern_len_bits));
    MOCK_METHOD3(GetPacketMinFrameBitCount, int(const Ch10PCMTMATSData& tmats,
        const PCMF1CSDWFmt* hdr, const int& pkt_sync_pattern_bits));
};

class MockCh10PCMF1Calculations: public Ch10PCMF1Calculations
{
   public:
    MockCh10PCMF1Calculations() : Ch10PCMF1Calculations() {}
    MOCK_METHOD6(CalculateMinorFrameCount, bool(
        Ch10PCMF1MinorFrame* minframe, 
        const uint32_t& pkt_data_sz,  
        const Ch10PCMTMATSData& tmats, const PCMF1CSDWFmt* hdr, 
        uint32_t& minor_frame_count, uint32_t& minor_frame_size));
};



#endif  // CH10_PCMF1_COMPONENT_MOCK_H_