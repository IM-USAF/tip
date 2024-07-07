
#ifndef CH10_PACKET_H_
#define CH10_PACKET_H_

#include <string>
#include <vector>
#include "ch10_context.h"
#include "ch10_time.h"
#include "ch10_1553f1_component.h"
#include "ch10_arinc429f0_component.h"
#include "ch10_videof0_component.h"
#include "ch10_ethernetf0_component.h"
#include "binbuff.h"
#include "ch10_status.h"
#include "ch10_packet_header_component.h"
#include "ch10_tmats_component.h"
#include "ch10_tdp_component.h"

class Ch10Packet
{
   private:
    const uint8_t* data_ptr_;

    // Hold pointers to BinBuff and Ch10Context to avoid
    // the need to pass them into the Parse function each time
    // a packet is parsed. Focus on performance.
    // Both objects will need to be updated
    // as packets are parsed.
    BinBuff* const bb_;
    Ch10Context* const ctx_;

    // Member variable to hold the BinBuff object function responses.
    uint8_t bb_response_;

    // Hold Ch10Status, avoid creating new variable each time.
    Ch10Status status_;

    // Hold pkt_size temporarily to allow passing by reference.
    uint64_t temp_pkt_size_;

    // Track packet type if a packet is being parsed.
    Ch10PacketType pkt_type_;

    // Secondary header time, relative time in nanoseconds.
    // Note the assumption of relative time is not confirmed.
    uint64_t secondary_hdr_time_ns_;

    // Various packet parser instances.
    Ch10PacketHeaderComponent* header_;
    Ch10TMATSComponent* tmats_;
    Ch10TDPComponent* tdp_component_;
    Ch101553F1Component* milstd1553f1_component_;
    Ch10VideoF0Component* videof0_component_;
    Ch10EthernetF0Component* ethernetf0_component_;
    Ch10429F0Component* arinc429f0_component_;

    // Ch10 time calculation and manipulation
    Ch10Time* const ch10_time_;

   public:


    const Ch10PacketType& current_pkt_type;
    Ch10Packet(BinBuff* const binbuff, Ch10Context* const context,
        Ch10Time* const ch10time) :
        ch10_time_(ch10time), secondary_hdr_time_ns_(0), bb_(binbuff), ctx_(context), data_ptr_(nullptr), bb_response_(0), status_(Ch10Status::OK), temp_pkt_size_(0), pkt_type_(Ch10PacketType::NONE), current_pkt_type(pkt_type_), header_(nullptr), tmats_(nullptr), tdp_component_(nullptr), milstd1553f1_component_(nullptr), arinc429f0_component_(nullptr), videof0_component_(nullptr), ethernetf0_component_(nullptr)
    {    }


    /*
    Set the Ch10 component parser pointers

    Args:
        header_comp     --> Pointer to Ch10PacketHeaderComponent
        tmats_comp      --> Pointer to Ch10TMATSComponent
        tdp_comp        --> Pointer to Ch10TDPComponent
        milstd1553_comp --> Pointer to Ch101553F1Component
        video_comp      --> Pointer to Ch10VideoF0Component
        eth_comp        --> Pointer to Ch10EthernetF0Component
        arinc429_comp   --> Pointer to Ch10429F0Component
    */
    void SetCh10ComponentParsers(Ch10PacketHeaderComponent* header_comp, Ch10TMATSComponent* tmats_comp,
        Ch10TDPComponent* tdp_comp, Ch101553F1Component* milstd1553_comp,
        Ch10VideoF0Component* video_comp, Ch10EthernetF0Component* eth_comp,
        Ch10429F0Component* arinc429_comp);



    /*
    Check if object has been properly configured. Currently this includes
    confirming that all pointers have been set and are no longer null pointers.

    Additional checks may be necessary in the future.

    For testing, use in conjunction with the constructor and
    SetCh10ComponentParsers.

    Return:
        True if configured properly, false otherwise.
    */
    bool IsConfigured();



    /*
    Parse the ch10 header at the current location of the buffer.

    Advances the buffer to the next potentially viable position.
    */
    Ch10Status ParseHeader();



    /*
    Maintain the logic for Ch10PacketHeaderComponent::Parse status. Header
    parsing outcome has great impact on the next parsing steps. Encapsulate
    the logic for the various status possibilities in this function.

    Args:

        status --> Ch10Status returned from Ch10PacketHeaderComponent::Parse()

        pkt_size --> Size of ch10 packet as given by pkt_size field in the
            Ch10PacketHeaderFmt.

    Return:

        Ch10Status --> Two classes of returns, good and bad. Good is indicated
            by Ch10Status::OK, and bad is any other status. A good indication
            implies that parsing of the header should continue and bad indicates
            that parsing of the header should stop and the status returned
            to the calling function.
    */
    Ch10Status ManageHeaderParseStatus(const Ch10Status& status, const uint64_t& pkt_size);



    /*
    Maintain the logic for Ch10PacketHeaderComponent::ParseSecondaryHeader
    status. Header
    parsing outcome has great impact on the next parsing steps. Encapsulate
    the logic for the various status possibilities in this function.

    Args:

        status --> Ch10Status returned from Ch10PacketHeaderComponent::Parse()

        pkt_size --> Size of ch10 packet as given by pkt_size field in the
            Ch10PacketHeaderFmt.

    Return:

        Ch10Status --> Two classes of returns, good and bad. Good is indicated
            by Ch10Status::OK, and bad is any other status. A good indication
            implies that parsing of the header should continue and bad indicates
            that parsing of the header should stop and the status returned
            to the calling function.
    */
    Ch10Status ManageSecondaryHeaderParseStatus(const Ch10Status& status,
                                                const uint64_t& pkt_size);




    /*
    Move position of buffer and absolute position index by amount indicated.
    Check for error response from BinBuff object.

    Args:

        byte_count --> Count of bytes to advance buffer and absolute position.
                       Input data type is uint32_t because the most common use
                       of this function will be to advance by the packet size
                       to move from the beginning of the current packet to the
                       beginning of the next packet. This value is parsed out
                       the ch10 packet header as a type uint32_t. This will
                       avoid casting during the vast majority of calls.
    */
    Ch10Status AdvanceBuffer(const uint64_t& byte_count);



    /*
    The primary purpose of this function is to call the correct parser
    for the packet type indicated in the header.

 	Args:
		abs_pos 	--> File read starting position in bytes of
						the current worker
		found_tmats	--> Boolean indicates if at least one tmats 
						packet has been parsed. VAlue is also set
						by this function.
    Return:
        Ch10Status
    */
    Ch10Status ParseBody(const uint64_t& abs_pos, bool& found_tmats);


	/*
	Logic for deciding if tmats has been read completely by the 
	first worker or if an error has occurred. 

	Args:
		abs_pos 	--> File read starting position in bytes of
						the current worker
		found_tmats	--> Boolean indicates if at least one tmats 
						packet has been parsed. VAlue is also set
						by this function.
        curr_tmats  --> Boolean indicates if the current packet
                        header is tmats 
		
	Return:
        Ch10Status: OK = continue parsing as normal, TMATS_PKT =
        TMATS packet found and no current worker should stop parsing, 
        TMATS_PKT_ERR = error related to TMATS.
	*/
	Ch10Status TmatsStatus(const uint64_t& abs_pos, bool& found_tmats, 
        bool curr_tmats);


};

#endif