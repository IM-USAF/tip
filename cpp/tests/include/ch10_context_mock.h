#ifndef CH10_CONTEXT_MOCK_H_
#define CH10_CONTEXT_MOCK_H_

#include <set>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_context.h"


class MockCh10Context: public Ch10Context
{
   public:
    MockCh10Context() : Ch10Context() {}
    MOCK_METHOD3(UpdateContext, Ch10Status(const uint64_t& abs_pos,
        const Ch10PacketHeaderFmt* const hdr_fmt_ptr_, const uint64_t& rtc_time));
    MOCK_METHOD1(UpdateWithSecondaryHeaderTime, void(const uint64_t& time_ns));
    MOCK_METHOD1(ContinueWithPacketType, Ch10Status(uint8_t data_type));
    MOCK_METHOD1(AdvanceAbsPos, void(uint64_t advance_bytes));
    MOCK_METHOD1(IsPacketTypeEnabled, bool(const Ch10PacketType& pkt_type));
    MOCK_METHOD1(RecordMinVideoTimeStamp, void(const uint64_t& ts));
    MOCK_METHOD1(RegisterUnhandledPacketType, bool(const Ch10PacketType& pkt_type));
    MOCK_CONST_METHOD0(GetChannelIDToRemoteAddr1Map, std::map<uint32_t, std::set<uint16_t>>());
    MOCK_CONST_METHOD0(GetChannelIDToRemoteAddr2Map, std::map<uint32_t, std::set<uint16_t>>());
    MOCK_CONST_METHOD0(GetChannelIDToCommWordsMap, std::map<uint32_t, std::set<uint32_t>>());
    MOCK_CONST_METHOD0(GetChannelIDToMinVideoTimestampMap, std::map<uint16_t, uint64_t>());
    MOCK_CONST_METHOD0(GetChannelIDToLabelsMap, std::map<uint32_t, std::set<uint16_t>>());
    MOCK_CONST_METHOD0(GetChannelIDToBusNumbersMap, std::map<uint32_t, std::set<uint16_t>>());
    MOCK_CONST_METHOD0(GetTMATSMatter, std::string());
    MOCK_CONST_METHOD0(GetParsedPacketTypes, const std::set<Ch10PacketType>&());
    MOCK_CONST_METHOD0(GetPacketDataSizeBytes, const uint32_t&());
};


#endif  // CH10_CONTEXT_MOCK_H_