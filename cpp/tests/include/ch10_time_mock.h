#ifndef CH10_TIME_MOCK_H_
#define CH10_TIME_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_time.h"

class MockCh10Time: public Ch10Time
{
   public:
    MockCh10Time() : Ch10Time() {}
    MOCK_METHOD2(CalculateRTCTimeFromComponents, uint64_t&(const uint32_t& rtc1, 
        const uint32_t& rtc2));
    MOCK_METHOD4(ParseIPTS, Ch10Status(const uint8_t*& data, uint64_t& time_ns,
                                 uint8_t intrapkt_ts_src, uint8_t time_fmt));
};

#endif  // CH10_TIME_MOCK_H_