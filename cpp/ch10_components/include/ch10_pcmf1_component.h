
#ifndef CH10_PCMF1_COMPONENT_H_
#define CH10_PCMF1_COMPONENT_H_

#include <cstdint>
#include <set>
#include "ch10_pcmf1_msg_hdr_format.h"
#include "ch10_packet_component.h"
#include "ch10_time.h"

// Value of IPDH length
static constexpr int IPTS_len_bytes = 8;


class Ch10PCMF1MinorFrame
{
    private:

    public:
        Ch10PCMF1MinorFrame() {};

    /*
    Get the count of bytes required to express the sync pattern, 
    based on sync pattern length. Several variations of expression
    exist. Note that the sync pattern lenth has been 
    tested for validity in tmats_data.cpp so it is unnecessary here.

    Note: Choose to return bits here and not bytes or words because
    there may not be an integer count of 16-bit words in packed mode.

    Args:
        hdr                     --> PCMF1CSDWFmt object
        sync_pattern_len_bits   --> Length of sync pattern in bits
                                    as configured in TMATS
    Return:
        Count of bytes to be read in order to interpret sync pattern.
    */
    virtual int GetPacketMinFrameSyncPatternBitCount(const PCMF1CSDWFmt* hdr, 
        const int& sync_pattern_len_bits);

    /*
    Get the count of bits in a minor frame which comprise all of the 
    data words, plus any pad words, and the sync pattern. Does not
    include IPH, if present.

    Args:
        tmats               --> Ch10PCMTMATSData object
        hdr                 --> PCMF1CSDWFmt object
        pkt_sync_pattern_bits --> Count of bits necessary which comprise
                                  the sync pattern, including any pad bits.
                                  This value is the return value of 
                                  GetPacketMinFrameSyncPatternBitCount.
    Return:
        Count of bits in minor frame
    */
    virtual int GetPacketMinFrameBitCount(const Ch10PCMTMATSData& tmats,
        const PCMF1CSDWFmt* hdr, const int& pkt_sync_pattern_bits);

    /*
    Handle lock status of minor or major frames. There is no prescription
    for handling data when lock is not active. Deal with IPDH lock status
    by skipping the current Ch10 PCMF1 packet given a certain combination
    of status or simply indicate with warning messages.  
    */

};


class Ch10PCMF1Calculations
{
    private:

        Ch10Status status_;

        // Time obtained from IPTS, nanosecond units
        uint64_t ipts_time_;

        // Length of the sync pattern as recorded in the packet, which includes
        // padding, etc. 
        int pkt_sync_pattern_len_bytes_;

        // Length of minor frame was recorded in the packet, dependent on 
        // the alignment size and other factors, and including the sync
        // pattern.
        int pkt_minor_frame_len_bytes_;

        int IPDH_len_bytes_;

    public:
        Ch10PCMF1Calculations() : pkt_sync_pattern_len_bytes_(-1), 
            pkt_minor_frame_len_bytes_(-1), IPDH_len_bytes_(-1), ipts_time_(0),
            status_(Ch10Status::NONE)
        {}
        const int& GetPktSyncPatternLenBytes() const { return pkt_sync_pattern_len_bytes_; }
        const int& GetPktMinFrameLenBytes() const { return pkt_minor_frame_len_bytes_; }
        const int& GetIPDHLenBytes() const { return IPDH_len_bytes_; }


    /*
    Determine count of minor frames and effective minor frame size, 
    given word alignment implications. Not applicable in throughput 
    mode. Note that this function should not be called for throughput
    mode; it is irrelevant (verify this understanding!).

    Args:
        minframe            --> Ch10PCMF1MinorFrame object
        pkt_data_sz         --> PCM F1 packet data size, in bytes
        tmats               --> Ch10PCMTMATSData object
        hdr                 --> PCMF1CSDWFmt object
        minor_frame_count     --> Count of minor frames in the packet,
                                to be assigned
        minor_frame_size      --> Aligned minor frame size in bytes, 
                                to be assigned

    Return:
        False if an error occurs, otherwise true.
    */
    virtual bool CalculateMinorFrameCount(Ch10PCMF1MinorFrame* minframe, 
        const uint32_t& pkt_data_sz,  
        const Ch10PCMTMATSData& tmats, const PCMF1CSDWFmt* hdr, 
        uint32_t& minor_frame_count, uint32_t& minor_frame_size);

    
    /*
    Parse and calculate the IPTS.

    Args:
		data		    --> pointer to the first byte in the series of
						    messages
        ch10time        --> Ch10Time object
        ctx             --> Ch10Context object
        abs_time_ns     --> Absolute time in nanoseconds, the primary
                            value of interest to be obtained.

    Return:
        Ch10status indicating OK if no issues, other status otherwise.
    */
    Ch10Status CalculateAbsTime(const uint8_t*& data, Ch10Time* const ch10time, 
        Ch10Context* const ctx, uint64_t& abs_time_ns);
};



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
        calcs       --> Ch10PCMF1Calculations object
		data		--> pointer to the first byte in the series of
						messages
        tmats       --> Ch10PCMTMATSData object containing PCM configuration
                        relevant to current PCM packet
        hdr         --> PCMF1CSDWFmt object
        ctx         --> Ch10Context object
        ch10time    --> Ch10Time object

	Return:
		Ch10Status::OK if no problems, otherwise a different Ch10Status code.
	*/
    Ch10Status ParseFrames(Ch10PCMF1Calculations* calcs, 
        const uint8_t*& data, const Ch10PCMTMATSData& tmats,
        const PCMF1CSDWFmt* hdr, Ch10Context* const ctx, Ch10Time* const ch10time);

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

};



#endif