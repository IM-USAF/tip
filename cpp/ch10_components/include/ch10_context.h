
#ifndef CH10_CONTEXT_H_
#define CH10_CONTEXT_H_

// Important to include the following two
// headers prior to others which include spdlog.h.
// There is a redefinition error with
// some of the types defined in Arrow.
#include "parquet_context.h"
#include "parquet_milstd1553f1.h"
#include "parquet_videodataf0.h"
#include "parquet_ethernetf0.h"
#include "parquet_arinc429f0.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <set>
#include <cmath>
#include <memory>
#include "managed_path.h"
#include "ch10_packet_type.h"
#include "ch10_status.h"
#include "ch10_header_format.h"
#include "ch10_1553f1_msg_hdr_format.h"
#include "ch10_videof0_header_format.h"
#include "ch10_arinc429f0_msg_hdr_format.h"
#include "ch10_tdpf1_hdr_format.h"
#include "ch10_pcm_tmats_data.h"
#include "spdlog/spdlog.h"

#include <fstream>


class Ch10Context
{
   private:
    using pcmdata_map = std::map<int, Ch10PCMTMATSData>;

    // ID to control or generate thread-specific log output.
    uint16_t thread_id_;

    // Key components of Ch10 context.
    uint64_t absolute_position_;
    uint64_t tdp_rtc_;       // nanosecond
    uint64_t tdp_abs_time_;  // nanosecond
    uint64_t rtc_;           // nanosecond
    uint32_t pkt_size_;
    uint32_t data_size_;

    // Temporary abs time for return by ref
    uint64_t temp_abs_time_;

    // Temporary RTC time in nanosecond
    uint64_t temp_rtc_;

    // If a secondary header is present, then absolute time
    // does not need to be calculated from the TDP, it is given
    // directly from the secondary header time. This variable
    // holds the absolute secondary header time.
    uint64_t packet_abs_time_;  // nanosecond

    // Reference map for packet type on/off state.
    // This map must include all elements of Ch10PacketType to
    // ensure that all types can be configured for on/off correctly.
    std::unordered_map<Ch10PacketType, bool> pkt_type_config_map_;

    // Store status of enabled file writers.
    std::unordered_map<Ch10PacketType, bool> pkt_type_file_writers_enabled_map_;

    // Store output paths of enabled file writers.
    std::unordered_map<Ch10PacketType, ManagedPath> pkt_type_paths_enabled_map_;

    bool searching_for_tdp_;
    bool found_tdp_;

    // Indicates that tdp data are relevant. This is only
    // valid after found_tdp_ is set to true. If this var
    // is false, then tdp data are not available, absolute time
    // can't be computed, and rtc time ought to be used in
    // place of absolute time.
    bool tdp_valid_;

    // 1 if IRIG time (day of year) is found in the TDP packet.
    // 0 if otherwise (day, month, year). This value is only
    // relevant after the TDP has been parsed, i.e., found_tdp_
    // is set to true.
    uint8_t tdp_doy_;

    // Intra-packet time stamp source, from Ch10PacketHeaderFmt::intrapkt_ts_source.
    // 0 = intra-packet TS source is the header RTC and should be parsed as RTC.
    // 1 = intra-packet TS source is the header secondary time stamp and should
    // be parsed as indicated by Ch10PacketHeaderFmt::time_fmt.
    uint8_t intrapkt_ts_src_;

    // Indicates the ch10 packet header secondary header time format,
    // if present (indicated by Ch10PacketHeaderFmt::secondary_hdr = 1)
    uint8_t time_format_;

    // Indicates the presence of a secondary header in the ch10 packet
    // header if value is 1, otherwise zero. If present, any intra-packet
    // time stamps will use the secondary header time as the time source.
    // This bit must be high if intrapkt_ts_src_ is 1.
    uint8_t secondary_hdr_;

    // Channel ID from Ch10PacketHeaderFmt (chanID), identifies data
    // source for the contents of the packet.
    uint32_t channel_id_;

    // Record mapping of 1553 message channel ID to lru addresses.
    std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr1_map_;
    std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr2_map_;

    // Record a 32-bit integer calculated from the command word(s).
    // Used for bus mapping. Map channel id to set of these command
    // word values.
    std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map_;

    // Record mapping of ARINC 429 channel ID to word labels.
    std::map<uint32_t, std::set<uint16_t>> chanid_labels_map_;

    // Record mapping of ARINC 429 channel ID to IPDH bus number.
    std::map<uint32_t, std::set<uint16_t>> chanid_busnumbers_map_;

    // Record mapping of channel id to IPDH bus number to labels on bus
    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> chanid_busnumbers_labels_map_;

    // Track the minimum (earliest) video timestamp per channel ID
    std::map<uint16_t, uint64_t> chanid_minvideotimestamp_map_;

    // Reference command words 1 and 2 in the 1553
    // message data header.
    const uint16_t* command_word1_;
    const uint16_t* command_word2_;

    // Set to true if CheckConfiguration has been executed
    // and its return value was true.
    bool is_configured_;

    //
    // File writers are owned by Ch10Context to maintain state
    //
    std::unique_ptr<ParquetContext> milstd1553f1_pq_ctx_;
    std::unique_ptr<ParquetMilStd1553F1> milstd1553f1_pq_writer_;
    std::unique_ptr<ParquetContext> videof0_pq_ctx_;
    std::unique_ptr<ParquetVideoDataF0> videof0_pq_writer_;
    std::unique_ptr<ParquetContext> ethernetf0_pq_ctx_;
    std::unique_ptr<ParquetEthernetF0> ethernetf0_pq_writer_;
    std::unique_ptr<ParquetContext> arinc429f0_pq_ctx_;
    std::unique_ptr<ParquetARINC429F0> arinc429f0_pq_writer_;

    // Internal state to be used in RegisterUnhandledPacketType()
    std::set<Ch10PacketType> registered_unhandled_packet_types_;

    // Set of packet types which are in the pkt_type_config_map_
    // and enabled and which are queried for config
    // status in IsPacketTypeEnabled(). This set is used to
    // identify the packet types which were parsed by the worker
    // associated with this Ch10Context instance.
    std::set<Ch10PacketType> parsed_packet_types_;

    // Record of each parsed TDP packet with generated
    // absolute time.
    std::vector<TDF1CSDWFmt> tdf1csdw_vec_;
    std::vector<uint64_t> tdp_abs_time_vec_;

    // Hold TMATS matter for later recording
    std::string tmats_matter_;

    // Map of tmats PCM attributes common index to
    // Ch10PCMTMATSData object populated with tmats
    // attributes which share the index.
    pcmdata_map pcm_tmats_data_map_;

   public:
    const uint16_t& thread_id;
    const uint64_t& absolute_position;
    const uint64_t& tdp_rtc;
    const uint64_t& tdp_abs_time;
    const uint64_t& rtc;
    const uint32_t& pkt_size;
    const uint32_t& data_size;
    const bool& tdp_valid;
    const uint8_t& tdp_doy;
    const bool& found_tdp;
    const uint8_t& intrapkt_ts_src;
    const uint8_t& time_format;
    const uint8_t& secondary_hdr;
    const uint32_t& channel_id;
    const std::unordered_map<Ch10PacketType, bool>& pkt_type_config_map;
    const std::unordered_map<Ch10PacketType, ManagedPath>& pkt_type_paths_map;
    const std::map<uint32_t, std::set<uint16_t>>& chanid_remoteaddr1_map;
    const std::map<uint32_t, std::set<uint16_t>>& chanid_remoteaddr2_map;
    const std::map<uint32_t, std::set<uint32_t>>& chanid_commwords_map;
    const std::map<uint32_t, std::set<uint16_t>>& chanid_labels_map;
    const std::map<uint32_t, std::set<uint16_t>>& chanid_busnumbers_map;
    const std::map<uint16_t, uint64_t>& chanid_minvideotimestamp_map;
    ParquetMilStd1553F1* milstd1553f1_pq_writer;
    ParquetVideoDataF0* videof0_pq_writer;
    ParquetEthernetF0* ethernetf0_pq_writer;
    ParquetARINC429F0* arinc429f0_pq_writer;
    const uint32_t intrapacket_ts_size_ = sizeof(uint64_t);
    const std::set<Ch10PacketType>& parsed_packet_types;
    const std::vector<TDF1CSDWFmt>& tdf1csdw_vec;
    const std::vector<uint64_t>& tdp_abs_time_vec;
    const pcmdata_map& pcm_tmats_data_map;

    Ch10Context(const uint64_t& abs_pos, uint16_t id = 0);
    Ch10Context();
    void Initialize(const uint64_t& abs_pos, uint16_t id);
    virtual ~Ch10Context();
    virtual std::map<uint32_t, std::set<uint16_t>> GetChannelIDToRemoteAddr1Map() const
    { return chanid_remoteaddr1_map_; }
    virtual std::map<uint32_t, std::set<uint16_t>> GetChannelIDToRemoteAddr2Map() const
    { return chanid_remoteaddr2_map_; }
    virtual std::map<uint32_t, std::set<uint32_t>> GetChannelIDToCommWordsMap() const
    { return chanid_commwords_map_; }
    virtual std::map<uint16_t, uint64_t> GetChannelIDToMinVideoTimestampMap() const
    { return chanid_minvideotimestamp_map_; }
    virtual std::map<uint32_t, std::set<uint16_t>> GetChannelIDToLabelsMap() const
    { return chanid_labels_map_; }
    virtual std::map<uint32_t, std::set<uint16_t>> GetChannelIDToBusNumbersMap() const
    { return chanid_busnumbers_map_; }
    virtual std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> GetChannelIDToBusNumbersToLabelsMap() const
    { return chanid_busnumbers_labels_map_; }
    virtual std::string GetTMATSMatter() const { return tmats_matter_; }
    virtual void AddTMATSMatter(const std::string& matter) { tmats_matter_ += matter; }

    virtual const std::set<Ch10PacketType>& GetParsedPacketTypes() const { return parsed_packet_types_; }


    /*
	Return is_configured_. This value is set during call to CheckConfiguration.
	If true, then CheckConfiguration has been called, presumably with the
	maps set during calls to SetPacketTypeConfig and SetOutputPathsMap and
	all necessary data are present and ready for parsing.
	*/
    bool IsConfigured();

    void SetPCMTMATSData(const pcmdata_map& pcm_data);
    void SetSearchingForTDP(bool should_search);
    virtual Ch10Status ContinueWithPacketType(uint8_t data_type);

    /*
    Check if input packet type is enabled.

    This function also curates a set of Ch10PacketType,
    parsed_packet_types_, to which a type is inserted if
    the input to the function is in the pkt_type_config_map_ and
    the is configured to true, i.e., to be parsed.

    Args:
        pkt_type    --> Ch10PacketType to be checked

    Return:
        True if enabled; false otherwise.
    */
    virtual bool IsPacketTypeEnabled(const Ch10PacketType& pkt_type);



    /*
    Mark the input un-handled Ch10PacketType as being found in the data.
    Used to prevent many log entries indicating that a given packet type
    is not handled. Unhandled packets ought to call this function
    and print to log only if it returns true.
    Args:
        pkt_type    --> Ch10PacketType to be registered

    Return:
        True if input pkt_type has not been found and
        registered via this function; false otherwise.
    */
    virtual bool RegisterUnhandledPacketType(const Ch10PacketType& pkt_type);



    /*
	Advance the absolute position by advance_bytes.

	Args:
		advance_bytes	--> Count of bytes by which to advance/increase
							the absolute_position_
	*/
    virtual void AdvanceAbsPos(uint64_t advance_bytes);

    /*
	Update the members that are of primary importance for conveyance
	to the packet body parsers, including re-calculation of the current
	packet absolute time based on TDP abs time, RTC, and the current packet
	RTC.

	Use the chanID in Ch10PacketHeaderFmt to insert an empty set
	in the chanid to remote lru addresses maps.

	Args:

		abs_pos			--> absolute byte position within the Ch10
		hdr_fmt_ptr_	--> pointer to Ch10PacketHeaderFmt
		rtc_time		--> current RTC time in nanosecond units

	Return:
		Ch10Status value

	*/
    virtual Ch10Status UpdateContext(const uint64_t& abs_pos,
                             const Ch10PacketHeaderFmt* const hdr_fmt_ptr_, const uint64_t& rtc_time);

    void CreateDefaultPacketTypeConfig(std::unordered_map<Ch10PacketType, bool>& input);

    /*
	Use a user-input map of Ch10PacketType to bool to assemble the
	pkt_type_config_map_.

	Args:
		user_config		--> map of Ch10PacketType to bool. For the user-submitted
							example map,
								{Ch10PacketType::MILSTD1553_F1 --> true},
								{Ch10PacketType::VIDEO_DATA_F0 --> false}
							1553 will be parsed and video (type f0) will not be
							parsed. TMATS (computer generated data, format 1)
							and time data packets (time data f1) cannot be
							turned off. Data types that are not configured will
							default to true.
		default_config	--> The current default configuration, use
							::pkt_type_config_map. This input can't be modified
							because it is const, but provides a way to perform
							certain tests without allowing the user access to
							edit the data.

	Return:
		True if there are no issues and false if the default config is not
		up to date with the user config.
	*/
    bool SetPacketTypeConfig(const std::map<Ch10PacketType, bool>& user_config,
                             const std::unordered_map<Ch10PacketType, bool>& default_config);

    /*
	Update tdp_rtc_, tdp_abs_time_, tdp_doy_, tdp_valid_ and found_tdp_,
	member vars necessary
	for absolute time calculation for other data packets.

	Args:
		tdp_abs_time	--> absolute time in nanoseconds since the epoch
							as calculated from data retrieved from the
							TDP packet
		tdp_doy			--> bit value, 1 = day-of-year time (IRIG time),
							0 = year-mth-day time
		tdp_valid		--> true if tdp csdw time_fmt or src are none,
							false otherwise. Set to false if tdp data is invalid.
        tdf1csdw        --> Instance of TDF1CSDWFmt to append to internal
                            record

	*/
    void UpdateWithTDPData(const uint64_t& tdp_abs_time, uint8_t tdp_doy,
                           bool tdp_valid, const TDF1CSDWFmt& tdf1csdw);

    /*
	Calculate absolute time from RTC time format. This context
	must have been updated with UpdateWithTDPData prior to this call
	in order for absolute time to be calculated correctly.

	Args:
		current_rtc	--> Current RTC time in nanosecond units

	Return:
		Absolute time in nanoseconds since the epoch
	*/
    uint64_t& CalculateAbsTimeFromRTCFormat(const uint64_t& current_rtc);

    /*
	Calculate this packet's absolute time using the time data packet
	relative time counter (RTC) and absolute time, and the this packet's
	relative time counter.

	Return:
		Absolute time this packet was received in nanoseconds since the epoch
	*/
    uint64_t& GetPacketAbsoluteTimeFromHeaderRTC();

    /*
	Calculate and return the absolute time of an intra-packet time stamp (IPTS)
	using the input time.

	The only validated form of time is the non-secondary header time RTC-type
	IPTS. In this case we know it is relative time and therefore the time data
	packet relative time and absolute time are used to calculate the absolute
	time.

	Information from the Ch10 suggest *all* non-RTC source (i.e., secondary
	header time) time formats are absolute. In this case a calculation is
	not necessary.

	Args:
		ipts_time--> input variable which is set to the calculated absolute time
   		             value in units of nanosecond since the epoch

	Return:
		Absolute time in units of nanoseconds since the epoch
	*/
    virtual uint64_t& CalculateIPTSAbsTime(const uint64_t& ipts_time);

    /*
	Calculate and return the absolute time of an ARINC 429 word using the
    sum of the packet gap time fields and the packet time stamp.

	The only validated form of time is the non-secondary header time RTC-type
	IPTS. In this case we know it is relative time and therefore the time data
	packet relative time and absolute time are used to calculate the absolute
	time.

	Information from the Ch10 suggest *all* non-RTC source (i.e., secondary
	header time) time formats are absolute. In this case a calculation is
	not necessary.

	Args:
		total_gap_time--> input variable which is the sum of all Gap Time values
                          from a 429 Ch10 packet's IPDHs. Units of 0.1 ms.

	Return:
		Absolute time in units of nanoseconds since the epoch
	*/
    virtual uint64_t& Calculate429WordAbsTime(const uint64_t& total_gap_time);

    /*
	Update maps of channel ID to remote addresses 1 and 2 as obtained from
	the 1553 intra-packet data headers.

	Update the channel ID to command words map. The integer inserted into
	the set of uint32_t is the upshifted command word1 + command word2.
	*/
    void UpdateChannelIDToLRUAddressMaps(const uint32_t& chanid,
                                         const MilStd1553F1DataHeaderCommWordFmt* data_header);

    /*
	Update maps of channel ID to ARINC 429 Bus Number as obtained from
	the 429 intra-packet data headers.

	Update the channel ID to ARINC 429 labels map. The integer inserted into
	the set of uint32_t is the Label found in the ARINC message.
	*/
    void UpdateARINC429Maps(const uint32_t& chanid,
                                         const ARINC429F0MsgFmt* data_header);


    /*
	Check if the configurations for packet type and output paths are consistent.
	Return false if an enabled packet type does not have an output file specified.
	Generate a Ch10PacketType to ManagedPath map that is consistent with enabled
	packet types and the selection of types for which a mapped ManagedPath exists
	in the pkt_type_paths_config input map. The generated map is stored in map
	that's passed by reference,

	Args:
		pkt_type_enabled_config	--> Map of Ch10PacketType to bool. Use map set by call
									to SetPacketTypeConfig.
		pkt_type_paths_config	--> Map of Ch10PacketType to ManagedPath. Use map set
									by call to SetOutputPathsMap
		enabled_paths			--> Reference to map of Ch10PacketType to ManagedPath,
									the output of the configuration check. Only valid
									if the return of this function is true.

	Return:
		True if configuration is valid and key-value pairs inserted in enabled_paths
		map are to be used. False otherwise.
	*/
    bool CheckConfiguration(const std::unordered_map<Ch10PacketType, bool>& pkt_type_enabled_config,
                            const std::map<Ch10PacketType, ManagedPath>& pkt_type_paths_config,
                            std::map<Ch10PacketType, ManagedPath>& enabled_paths);

    /*
	Initialize file writers for the various enabled packet types. This function
	exists to avoid initializing file writers by default in the various parser
	constructors, which makes testing difficult.

	Instantiate file writers for each of the mapped types using the ManagedPath
	values.

	Args:
		enabled_paths	--> Const ref to map of Ch10PacketType to ManagedPath
							containing only types that have been enabled (SetPacketTypeConfig)
							by the user. This map is to be generated by CheckConfiguration.

    Return:
        0 if all writers are initialized succesfully. 
        /usr/include/sysexits.h otherwise.
	*/
    int InitializeFileWriters(const std::map<Ch10PacketType, ManagedPath>& enabled_paths);

    /*
	Close file writers for the various enabled packet types. Uses
	pkt_type_file_writers_enabled_map_, which is created during the call to
	InitializeFileWriters and stored as a private member var.
	*/
    void CloseFileWriters() const;

    /*
	Submit a video timestamp, relative to the current channel ID (channel_id_),
	to be compared against previously submitted timestamps with same channel ID.
	The minimum of the current timestamp and the previously submitted timestamp
	will be recorded such that only the minimum of all timestamps for a given
	channel ID will be recorded.

	Args:
		ts		--> Video timestamp in nanosecond unit, counting from the
					unix epoch
	*/
    virtual void RecordMinVideoTimeStamp(const uint64_t& ts);

    /*
	Set the current packet secondary header time, which is
	absolute time.

	Args:
		time_ns		--> Secondary header time, nanosecond unit
	*/
    virtual void UpdateWithSecondaryHeaderTime(const uint64_t& time_ns);

};

#endif
