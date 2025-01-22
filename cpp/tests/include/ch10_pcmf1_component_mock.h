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
    MOCK_METHOD2(GetPacketMinFrameSyncPatternBitCount, int(
        const PCMF1CSDWFmt* hdr, const int& sync_pattern_len_bits));
    MOCK_METHOD3(GetPacketMinFrameBitCount, int(const Ch10PCMTMATSData& tmats,
        const PCMF1CSDWFmt* hdr, const int& pkt_sync_pattern_bits));
};


#endif  // CH10_PCMF1_COMPONENT_MOCK_H_