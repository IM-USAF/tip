
#ifndef CH10_429F0_COMPONENT_H_
#define CH10_429F0_COMPONENT_H_

#include <cstdint>
#include <set>
#include "ch10_arinc429f0_msg_hdr_format.h"
#include "ch10_packet_component.h"
#include "ch10_time.h"



/*
This class defines the structures/classes and methods
to parse Ch10 "ARINC 429 Bus Data Packets, Format 0".
*/

class Ch10429F0Component : public Ch10PacketComponent
{
   private:
    Ch10PacketElement<ARINC429F0CSDWFmt> arinc429f0_csdw_elem_;
    Ch10PacketElement<ARINC429F0MsgFmt> arinc429f0_data_hdr_wrd_elem_;

    ElemPtrVec arinc429f0_csdw_elem_vec_;
    ElemPtrVec arinc429f0_data_hdr_wrd_elem_vec_;

    const ARINC429F0MsgFmt* arinc429f0_msg_fmt_ptr_;

    // Index of the ARINC 429 word being parsed within the ch10
    // packet.
    uint32_t msg_index_;

    // Hold absolute time of 429 word being parsed in units of nanoseconds
    // since the epoch.
    uint64_t abs_time_;

    // Time obtained from IP header's gap time, nanosecond units
    uint64_t ip_gap_time_;

    //
    // Vars for parsing the ARINC 429 word payloads
    //

    Ch10Time ch10_time_;

   public:
    const uint64_t& abs_time;
    // const uint16_t* const* const payload_ptr_ptr;

    const Ch10PacketElement<ARINC429F0CSDWFmt>& arinc429f0_csdw_elem;
    const Ch10PacketElement<ARINC429F0MsgFmt>& arinc429f0_data_hdr_wrd_elem;

    Ch10429F0Component(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx),
                                                      arinc429f0_csdw_elem_vec_{
                                                          dynamic_cast<Ch10PacketElementBase*>(&arinc429f0_csdw_elem_)},
                                                      arinc429f0_data_hdr_wrd_elem_vec_{
                                                          dynamic_cast<Ch10PacketElementBase*>(&arinc429f0_data_hdr_wrd_elem_)},
                                                      arinc429f0_csdw_elem(arinc429f0_csdw_elem_),
                                                      arinc429f0_data_hdr_wrd_elem(arinc429f0_data_hdr_wrd_elem_),
                                                      msg_index_(0),
                                                      abs_time_(0),
                                                      abs_time(abs_time_),
                                                      ch10_time_()

    {
    }

    Ch10Status Parse(const uint8_t*& data) override;

    /*
	Parse all of the ARINC429 words in the body of the ch10 429 packet that
	follows the CSDW. Each word is composed of an intra-packet-data-header,
    followed by 32-bit data word with label, SDI, payload, etc. This
	function parses intra-packet matter and the data word for all ARINC
	words. Using the packet time stamp and IPDH's Time Gap field,
	it also sets the private member var abs_time_.

	Args:
		msg_count	--> count of word, each with header containing gap time,
						in the packet
		data		--> pointer to the first byte in the series of
						messages

	Return:
		Ch10Status::OK if no problems, otherwise a different Ch10Status code.
	*/
    Ch10Status ParseMessages(const uint32_t& msg_count, const uint8_t*& data);

};

#endif