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


#endif  // CH10_PCMF1_COMPONENT_MOCK_H_