
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

    // Hold absolute time of current message in units of nanoseconds
    // since the epoch.
    uint64_t abs_time_;

    // Time obtained from IPTS, nanosecond units
    uint64_t ipts_time_;

    Ch10Time ch10_time_;

    //////////////////////////////////////////////////////////
    // Calculated values
    //////////////////////////////////////////////////////////
    int majframe_len_bits_;


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
        ipts_time_(0),
        majframe_len_bits_(-1)
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

    /*
    Sanity check on frame indicators. Both minor frame indicator (MI)
    and major frame indicator (MA) cannot simultaneously indicate 
    true = 1. Frame indicators are irrelevant for throughput mode. 

    Args:
        througphput --> 1 = throughput mode enabled 
        MI          --> 1 = first word in the packet is the beginning
                            of a minor frame
        MA          --> 1 = first word in the packet is the beginning
                            of a major frame

    Return:
        True if no conflicting configuration. False otherwise. 
    */
    bool CheckFrameIndicator(const uint32_t& throughput, const uint32_t& MI, 
        const uint32_t& MA);

    /*
    Determine count of minor frames and effective minor frame size, 
    given word alignment implications. Not applicable in throughput 
    mode. Note that this function should not be called for throughput
    mode; it is irrelevant (verify this understanding!).

    Args:
        pkt_data_sz         --> PCM F1 packet body size, in bytes
        tmats               --> Ch10PCMTMATSData object
        hdr                 --> PCMF1CSDWFmt object
        minor_frame_count     --> Count of minor frames in the packet,
                                to be assigned
        minor_frame_size      --> Aligned minor frame size in bytes, 
                                to be assigned

    Return:
        False if an error occurs, otherwise true.
    */
    bool CalculateMinorFrameCount(const uint32_t& pkt_data_sz,  
        const Ch10PCMTMATSData& tmats, const PCMF1CSDWFmt* hdr, 
        uint32_t& minor_frame_count, uint32_t& minor_frame_size);


    /*
    Get the count of bytes required to express the sync pattern, 
    based on sync pattern length. Several variations of expression
    exist. Note that currently, the sync pattern lenth has been 
    tested for validity in tmats_data.cpp.

    Args:
        hdr                     --> PCMF1CSDWFmt object
        sync_pattern_len_bits   --> Length of sync pattern in bits
                                    as configured in TMATS
    Return:
        Count of bytes to be read in order to interpret sync pattern.
    */
    int GetPacketMinFrameSyncPatternBitCount(const PCMF1CSDWFmt* hdr, 
        const int& sync_pattern_len_bits);

    /*
    Handle lock status of minor or major frames. There is no prescription
    for handling data when lock is not active. Deal with IPDH lock status
    by skipping the current Ch10 PCMF1 packet given a certain combination
    of status or simply indicate with warning messages.  
    */
};

#endif