
#ifndef CH10_PCMF1_COMPONENT_H_
#define CH10_PCMF1_COMPONENT_H_

#include <cstdint>
#include <set>
#include "ch10_pcmf1_msg_hdr_format.h"
#include "ch10_packet_component.h"
#include "ch10_time.h"


/*
This class defines the structures/classes and methods
to parse Ch10 "PCM Data, Format 1".
*/

class Ch10PCMF1Component : public Ch10PacketComponent
{
   private:
    Ch10PacketElement<PCMF1CSDWFmt> pcmf1_csdw_elem_;
    Ch10PacketElement<PCMF1IPDH16Fmt> pcmf1_data_hdr16_elem_;
    Ch10PacketElement<PCMF1IPDH32Fmt> pcmf1_data_hdr32_elem_;

    ElemPtrVec pcmf1_csdw_elem_vec_;
    ElemPtrVec pcmf1_ip_data_hdr16_elem_vec_;
    ElemPtrVec pcmf1_ip_data_hdr32_elem_vec_;

    const PCMF1CSDWModeFmt* pcmf1_data_hdr_mode_ptr_;
    const PCMF1CSDWLockstFmt* pcmf1_data_hdr_lockst_ptr_;
    
    // Hold absolute time of current message in units of nanoseconds
    // since the epoch.
    uint64_t abs_time_;

    // Time obtained from IPTS, nanosecond units
    uint64_t ipts_time_;

    Ch10Time ch10_time_;

   public:
    const uint64_t& abs_time;
    const Ch10PacketElement<PCMF1CSDWFmt>& pcmf1_csdw_elem;
    const Ch10PacketElement<PCMF1IPDH16Fmt>& pcmf1_data_hdr16_elem;
    const Ch10PacketElement<PCMF1IPDH32Fmt>& pcmf1_data_hdr32_elem;

    Ch10PCMF1Component(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx),
        pcmf1_csdw_elem_vec_{dynamic_cast<Ch10PacketElementBase*>(&pcmf1_csdw_elem_)},
        pcmf1_ip_data_hdr16_elem_vec_{dynamic_cast<Ch10PacketElementBase*>(&pcmf1_data_hdr16_elem_)},
        pcmf1_ip_data_hdr32_elem_vec_{dynamic_cast<Ch10PacketElementBase*>(&pcmf1_data_hdr32_elem_)},
        pcmf1_csdw_elem(pcmf1_csdw_elem_),
        pcmf1_data_hdr16_elem(pcmf1_data_hdr16_elem_),
        pcmf1_data_hdr32_elem(pcmf1_data_hdr32_elem_),
        abs_time_(0),
        abs_time(abs_time_),
        ch10_time_(),
        ipts_time_(0)
    { }

    Ch10Status Parse(const uint8_t*& data) override;
    

    /*
	Parse all of the messages in the body of the 1553 packet that
	follows the CSDW. Each message is composed of an intra-packet time
	stamp and a header, followed by n bytes of message payload. This
	function parses intra-packet matter and the message payload for
	all messages in the case of RTC format intra-packet time stamps.
	It also sets the private member var abs_time_.

	Args:
		msg_count	--> count of messages, each with time and header,
						in the packet
		data		--> pointer to the first byte in the series of
						messages

	Return:
		Ch10Status::OK if no problems, otherwise a different Ch10Status code.
	*/
    Ch10Status ParseFrames(const uint8_t*& data);

    // Ch10Status ParsePayload(const uint8_t*& data,
    //                         const PCMF1DataHeaderCommWordFmt* data_header);

    // uint16_t GetWordCountFromDataHeader(const PCMF1DataHeaderCommWordFmt* data_header);
};

#endif