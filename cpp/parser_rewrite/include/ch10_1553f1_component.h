
#ifndef CH10_1553F1_COMPONENT_H_
#define CH10_1553F1_COMPONENT_H_

#include <cstdint>
#include "ch10_1553f1_msg_hdr_format.h"
#include "ch10_packet_component.h"

class MilStd1553F1CSDWFmt
{
public:
	uint32_t count : 24;
	uint32_t : 6;
	uint32_t ttb : 2;
};

class MilStd1553F1DataRTCTimeStampFmt
{
public:
	uint64_t ts1_ : 32;
	uint64_t ts2_ : 32;
};

/*
This class defines the structures/classes and methods
to parse Ch10 "Mil-Std-1553 Bus Data Packets, Format 1".
*/

class Ch101553F1Component : public Ch10PacketComponent
{
private:

	Ch10PacketElement<MilStd1553F1CSDWFmt> milstd1553f1_csdw_elem_;
	Ch10PacketElement<MilStd1553F1DataRTCTimeStampFmt> milstd1553f1_rtctime_elem_;
	Ch10PacketElement<MilStd1553F1DataHeaderFmt> milstd1553f1_data_hdr_elem_;

	ElemPtrVec milstd1553f1_csdw_elem_vec_;
	ElemPtrVec milstd1553f1_rtctime_data_hdr_elem_vec_;

	// Index of the 1553 message being parsed within the ch10
	// 1553 packet.
	uint32_t msg_index_;

	// Hold absolute time of current message in units of nanoseconds
	// since the epoch.
	uint32_t abs_time_; 

	//
	// Vars for parsing the 1553 message payloads
	//

	// Max allowed message count within a 1553 packet. Used to
	// check for corruption issues. This value is very rough guess.
	const uint16_t max_message_count_;

	// Max allowed byte count per message payload.
	// max of (32 payload words + 2 command words + 2 status words) 
	// * 2 bytes per word = 72 bytes.
	const uint16_t max_byte_count_;

	// Interpret the raw bytes as uint16_t words
	const uint16_t* payload_ptr_;

	// 1 if payload word count from command word is greater than
	// the calculated payload word count from the message length,
	// 0 otherwise.
	uint8_t is_payload_incomplete_;

	// Calculated payload word count from the message length,
	// subtracting the command and status words. Signed integer
	// to allow negative values, which can occur when the count
	// of transmitted messages is in error or there are some
	// message errors.
	int8_t calc_payload_word_count_;

	// Expected payload word count, as interpreted from the intra-packet
	// header and taking into consideration that the current message may
	// be a mode code and that a word count of zero 
	// indicates a 32-word payload.
	uint16_t expected_payload_word_count_;


public:

	const Ch10PacketElement<MilStd1553F1CSDWFmt>& milstd1553f1_csdw_elem;
	const Ch10PacketElement<MilStd1553F1DataRTCTimeStampFmt>& milstd1553f1_rtctime_elem;
	const Ch10PacketElement<MilStd1553F1DataHeaderFmt>& milstd1553f1_data_hdr_elem;

	Ch101553F1Component(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx),
		milstd1553f1_csdw_elem_vec_{ 
			dynamic_cast<Ch10PacketElementBase*>(&milstd1553f1_csdw_elem_) },
		milstd1553f1_rtctime_data_hdr_elem_vec_{
			dynamic_cast<Ch10PacketElementBase*>(&milstd1553f1_rtctime_elem_),
			dynamic_cast<Ch10PacketElementBase*>(&milstd1553f1_data_hdr_elem_) },
		milstd1553f1_csdw_elem(milstd1553f1_csdw_elem_),
		milstd1553f1_rtctime_elem(milstd1553f1_rtctime_elem_),
		milstd1553f1_data_hdr_elem(milstd1553f1_data_hdr_elem_),
		msg_index_(0), abs_time_(0), max_message_count_(10000),
		payload_ptr_(nullptr), max_byte_count_(72), expected_payload_word_count_(0),
		calc_payload_word_count_(0), is_payload_incomplete_(0)
	{}
	Ch10Status Parse(const uint8_t*& data, uint64_t& loc) override;

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
		loc			--> counter to be incremented by the count
						of bytes processed/read during the parsing
						of the messages

	Return:
		Status of parsing
	*/
	Ch10Status ParseRTCTimeMessages(const uint32_t& msg_count, 
		const uint8_t*& data, uint64_t& loc);

	Ch10Status ParsePayload(const uint8_t*& data,
		const MilStd1553F1DataHeaderFmt* const data_header);

	uint16_t GetWordCountFromDataHeader(const MilStd1553F1DataHeaderFmt* const data_header);
};

#endif