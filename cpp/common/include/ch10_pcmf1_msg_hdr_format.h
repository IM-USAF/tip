
#ifndef CH10_PCMF1_MSG_HDR_FORMAT_H_
#define CH10_PCMF1_MSG_HDR_FORMAT_H_

#include <cstdint>

class PCMF1CSDWFmt
{
   public:

   // Word offset into the major frame for the first data word in 
   // the packet. Not applicable for packed or throughput mode. 
    uint32_t sync_offset : 18;

    // Indicates whether alignment, throughput, packed, or
    // unpacked mode -- mutually exclusive.
    // uint32_t mode : 6;
    uint32_t mode_unpacked : 1;
    uint32_t mode_packed : 1;
    uint32_t mode_throughput : 1;
    uint32_t mode_align : 1;  // 0 = 16-bit, 1 = 32-bit
    uint32_t : 2;

    // Lock status of the frame synchronizer. NA for throughput mode.
    // uint32_t lockst : 4;
    uint32_t majfstat : 2;
    uint32_t minfstat : 2;


    // Minor frame indicator. NA for throughput mode.
    uint32_t MI : 1;

    // Major fram indicator. NA for throughput mode.
    uint32_t MA : 1;

    // Determines if IPH is included or omitted. 
    uint32_t IPH : 1;
    uint32_t : 1;
};

class PCMF1IPDH16Fmt
{
   public:
    uint16_t : 12;
    uint16_t lockst : 4;
};

class PCMF1IPDH32Fmt
{
   public:
    uint32_t : 12;
    uint32_t lockst : 4;
};


#endif