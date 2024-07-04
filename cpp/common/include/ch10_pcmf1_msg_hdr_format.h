
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
    // unpacked mode.
    uint32_t mode : 6;

    // Lock status of the frame synchronizer. NA for throughput mode.
    uint32_t lockst : 4;

    // Minor frame indicator. NA for throughput mode.
    uint32_t MI : 1;

    // Major fram indicator. NA for throughput mode.
    uint32_t MA : 1;

    // Determines if IPH is included or omitted. 
    uint32_t IPH : 1;
    uint32_t : 1;
};

class PCMF1CSDWModeFmt
{
    public:
    
    uint8_t unpacked : 1;
    uint8_t packed : 1;
    uint8_t throughput : 1;

    // 0 = 16-bit alignment, 1 = 32-bit alignment 
    uint8_t alignment : 1;
};

class PCMF1CSDWLockstFmt
{
    public:

    // Major frame status: 0 = not locked, 2 = major frame check 
    // (after losing lock), 3 = major frame lock
    uint8_t majfstat : 2; 

    // Minor frame status: 2 = minor frame check (after losing lock), 
    // 3 = minor frame lock
    uint8_t minfstat : 2; 
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