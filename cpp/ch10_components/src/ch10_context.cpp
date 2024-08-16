#include "ch10_context.h"

Ch10Context::Ch10Context(const uint64_t& abs_pos, uint16_t id) : absolute_position_(abs_pos),
                                                                 absolute_position(absolute_position_),
                                                                 tdp_rtc_(0),
                                                                 tdp_rtc(tdp_rtc_),
                                                                 tdp_abs_time_(0),
                                                                 tdp_abs_time(tdp_abs_time_),
                                                                 searching_for_tdp_(false),
                                                                 found_tdp_(false),
                                                                 pkt_type_config_map(pkt_type_config_map_),
                                                                 pkt_size_(0),
                                                                 pkt_size(pkt_size_),
                                                                 data_size_(0),
                                                                 data_size(data_size_),
                                                                 secondary_hdr_(0),
                                                                 secondary_hdr(secondary_hdr_),
                                                                 temp_abs_time_(0),
                                                                 packet_abs_time_(0),
                                                                 rtc_(0),
                                                                 rtc(rtc_),
                                                                 thread_id_(id),
                                                                 thread_id(thread_id_),
                                                                 tdp_valid_(false),
                                                                 tdp_valid(tdp_valid_),
                                                                 tdp_doy_(0),
                                                                 tdp_doy(tdp_doy_),
                                                                 found_tdp(found_tdp_),
                                                                 intrapkt_ts_src_(0),
                                                                 intrapkt_ts_src(intrapkt_ts_src_),
                                                                 time_format_(0),
                                                                 time_format(time_format_),
                                                                 channel_id_(UINT32_MAX),
                                                                 channel_id(channel_id_),
                                                                 temp_rtc_(0),
                                                                 chanid_remoteaddr1_map(chanid_remoteaddr1_map_),
                                                                 chanid_remoteaddr2_map(chanid_remoteaddr2_map_),
                                                                 chanid_commwords_map(chanid_commwords_map_),
                                                                 chanid_labels_map(chanid_labels_map_),
                                                                 chanid_busnumbers_map(chanid_busnumbers_map_),
                                                                 command_word1_(nullptr),
                                                                 command_word2_(nullptr),
                                                                 is_configured_(false),
                                                                 milstd1553f1_pq_writer_(nullptr),
                                                                 milstd1553f1_pq_writer(nullptr),
                                                                 videof0_pq_writer_(nullptr),
                                                                 videof0_pq_writer(nullptr),
                                                                 ethernetf0_pq_writer_(nullptr),
                                                                 ethernetf0_pq_writer(nullptr),
                                                                 arinc429f0_pq_writer_(nullptr),
                                                                 arinc429f0_pq_writer(nullptr),
                                                                 chanid_minvideotimestamp_map(chanid_minvideotimestamp_map_),
                                                                 pkt_type_paths_map(pkt_type_paths_enabled_map_),
                                                                 milstd1553f1_pq_ctx_(nullptr),
                                                                 videof0_pq_ctx_(nullptr),
                                                                 parsed_packet_types(parsed_packet_types_),
                                                                 tdf1csdw_vec(tdf1csdw_vec_),
                                                                 tdp_abs_time_vec(tdp_abs_time_vec_),
                                                                 tmats_matter_(),
                                                                 pcm_tmats_data_map_(),
                                                                 pcm_tmats_data_map(pcm_tmats_data_map_)
{
    CreateDefaultPacketTypeConfig(pkt_type_config_map_);
}

Ch10Context::Ch10Context() : absolute_position_(0),
                             absolute_position(absolute_position_),
                             tdp_rtc_(0),
                             tdp_rtc(tdp_rtc_),
                             tdp_abs_time_(0),
                             tdp_abs_time(tdp_abs_time_),
                             searching_for_tdp_(false),
                             found_tdp_(false),
                             pkt_type_config_map(pkt_type_config_map_),
                             pkt_size_(0),
                             pkt_size(pkt_size_),
                             data_size_(0),
                             data_size(data_size_),
                             secondary_hdr_(0),
                             secondary_hdr(secondary_hdr_),
                             packet_abs_time_(0),
                             rtc_(0),
                             rtc(rtc_),
                             thread_id_(UINT16_MAX),
                             thread_id(thread_id_),
                             temp_abs_time_(0),
                             tdp_valid_(false),
                             tdp_valid(tdp_valid_),
                             tdp_doy_(0),
                             tdp_doy(tdp_doy_),
                             found_tdp(found_tdp_),
                             intrapkt_ts_src_(0),
                             intrapkt_ts_src(intrapkt_ts_src_),
                             time_format_(0),
                             time_format(time_format_),
                             channel_id_(UINT32_MAX),
                             channel_id(channel_id_),
                             temp_rtc_(0),
                             chanid_remoteaddr1_map(chanid_remoteaddr1_map_),
                             chanid_remoteaddr2_map(chanid_remoteaddr2_map_),
                             chanid_commwords_map(chanid_commwords_map_),
                             chanid_labels_map(chanid_labels_map_),
                             chanid_busnumbers_map(chanid_busnumbers_map_),
                             command_word1_(nullptr),
                             command_word2_(nullptr),
                             is_configured_(false),
                             milstd1553f1_pq_writer_(nullptr),
                             milstd1553f1_pq_writer(nullptr),
                             videof0_pq_writer_(nullptr),
                             videof0_pq_writer(nullptr),
                             ethernetf0_pq_writer_(nullptr),
                             ethernetf0_pq_writer(nullptr),
                             arinc429f0_pq_writer_(nullptr),
                             arinc429f0_pq_writer(nullptr),
                             chanid_minvideotimestamp_map(chanid_minvideotimestamp_map_),
                             pkt_type_paths_map(pkt_type_paths_enabled_map_),
                             parsed_packet_types(parsed_packet_types_),
                             tdf1csdw_vec(tdf1csdw_vec_),
                             tdp_abs_time_vec(tdp_abs_time_vec_),
                             tmats_matter_(),
                             pcm_tmats_data_map_(),
                             pcm_tmats_data_map(pcm_tmats_data_map_)
{
    CreateDefaultPacketTypeConfig(pkt_type_config_map_);
}


void Ch10Context::SetPCMTMATSData(const pcmdata_map& pcm_data)
{
    pcm_tmats_data_map_ = pcm_data;
}

bool Ch10Context::IsPacketTypeEnabled(const Ch10PacketType& pkt_type)
{
    if(pkt_type_config_map_.count(pkt_type) != 0)
    {
        if(pkt_type_config_map_.at(pkt_type))
        {
            parsed_packet_types_.insert(pkt_type);
            return true;
        }
    }
    return false;
}

void Ch10Context::Initialize(const uint64_t& abs_pos, uint16_t id)
{
    absolute_position_ = abs_pos;
    thread_id_ = id;
}

Ch10Context::~Ch10Context()
{
}

void Ch10Context::SetSearchingForTDP(bool should_search)
{
    searching_for_tdp_ = should_search;
    found_tdp_ = false;
}

Ch10Status Ch10Context::ContinueWithPacketType(uint8_t data_type)
{
    // If the boolean searching_for_tdp_ is true then return false unless
    // the current packet is a "TMATS" (computer generated data, format 1)
    // or TDP.
    if (searching_for_tdp_)
    {
        if (!found_tdp_)
        {
            if (data_type == static_cast<uint8_t>(Ch10PacketType::COMPUTER_GENERATED_DATA_F1))
            {
                SPDLOG_DEBUG("({:02d}) TMATS found", thread_id);
                return Ch10Status::PKT_TYPE_YES;
            }
            else if (data_type == static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1))
            {
                SPDLOG_DEBUG("({:02d}) TDP found", thread_id);
                found_tdp_ = true;
                return Ch10Status::PKT_TYPE_YES;
            }
            return Ch10Status::PKT_TYPE_NO;
        }
    }
    else
    {
        if (data_type == static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1))
        {
            return Ch10Status::PKT_TYPE_EXIT;
        }
    }

    return Ch10Status::PKT_TYPE_YES;
}

void Ch10Context::AdvanceAbsPos(uint64_t advance_bytes)
{
    absolute_position_ += advance_bytes;
}

Ch10Status Ch10Context::UpdateContext(
    const uint64_t& abs_pos,
    const Ch10PacketHeaderFmt* const hdr_fmt_ptr,
    const uint64_t& rtc_time)
{
    absolute_position_ = abs_pos;
    pkt_size_ = hdr_fmt_ptr->pkt_size;
    data_size_ = hdr_fmt_ptr->data_size;
    intrapkt_ts_src_ = hdr_fmt_ptr->intrapkt_ts_source;
    time_format_ = hdr_fmt_ptr->time_format;
    secondary_hdr_ = hdr_fmt_ptr->secondary_hdr;
    channel_id_ = hdr_fmt_ptr->chanID;
    rtc_ = rtc_time;

    // Time format is undefined or inconclusive if the intrapkt_ts_source = 0
    // and secondary_hdr = 1 or intrapkt_ts_source = 1 and secondary_hdr = 0.
    // It is not clear from the ch10 spec what value the time_format_ field
    // ought to take if intrapkt_ts_source = 0 so it is not tested here.
    if ((intrapkt_ts_src_ == 0 && secondary_hdr_ == 1) ||
        (intrapkt_ts_src_ == 1 && secondary_hdr_ == 0))
    {
        SPDLOG_ERROR(
            "({:02d}) intrapkt_ts_src_ = {:d} and secondary_hdr_ = {:d}, "
            "secondary header and intra-packet source inconclusive",
            thread_id, intrapkt_ts_src_, secondary_hdr_);
        return Ch10Status::TIME_FORMAT_INCONCLUSIVE;
    }

    // If the channel ID to remote LRU address maps don't have a mapping for the
    // current channel id, then add it, but only if the current packet type is
    // 1553.
    if (hdr_fmt_ptr->data_type == static_cast<uint8_t>(Ch10PacketType::MILSTD1553_F1))
    {
        if (chanid_remoteaddr1_map_.count(channel_id_) == 0)
        {
            std::set<uint16_t> temp_set;
            chanid_remoteaddr1_map_[channel_id_] = temp_set;
            chanid_remoteaddr2_map_[channel_id_] = temp_set;

            std::set<uint32_t> temp_set2;
            chanid_commwords_map_[channel_id_] = temp_set2;
        }
    }

    return Ch10Status::OK;
}

void Ch10Context::UpdateWithSecondaryHeaderTime(const uint64_t& time_ns)
{
    packet_abs_time_ = time_ns;
}

void Ch10Context::CreateDefaultPacketTypeConfig(std::unordered_map<Ch10PacketType, bool>& input)
{
    // Ensure that all elements defined in Ch10PacketType are present in the map
    // and initialized to true, or turned on by default.
    input[Ch10PacketType::COMPUTER_GENERATED_DATA_F1] = true;
    input[Ch10PacketType::TIME_DATA_F1] = true;
    input[Ch10PacketType::MILSTD1553_F1] = true;
    input[Ch10PacketType::VIDEO_DATA_F0] = true;
    input[Ch10PacketType::ETHERNET_DATA_F0] = true;
    input[Ch10PacketType::ARINC429_F0] = true;
    input[Ch10PacketType::PCM_F1] = true;
}

bool Ch10Context::SetPacketTypeConfig(const std::map<Ch10PacketType, bool>& user_config,
                                      const std::unordered_map<Ch10PacketType, bool>& default_config)
{
    // Loop over user map and only turn off entries that correspond
    // to the packet types that are set to false in the user map.
    using MapIt = std::map<Ch10PacketType, bool>::const_iterator;
    for (MapIt it = user_config.cbegin(); it != user_config.cend(); ++it)
    {
        // If the current user config Ch10PacketType is not in the default
        // config, alert the user and return false.
        if (default_config.count(it->first) == 0)
        {
            SPDLOG_ERROR(
                "({:02d}) The type {:s} can't be found in the "
                "default config. Update the default config for consistency. "
                "See Ch10Context::CreateDefaultPacketTypeConfig.",
                thread_id, ch10packettype_to_string_map.at(it->first));
            return false;
        }
        pkt_type_config_map_[it->first] = it->second;
    }

    // Regardless of current configuration after applying user config, set tmats
    // and time packets to true = on.
    pkt_type_config_map_[Ch10PacketType::COMPUTER_GENERATED_DATA_F1] = true;
    pkt_type_config_map_[Ch10PacketType::TIME_DATA_F1] = true;

    return true;
}

void Ch10Context::UpdateWithTDPData(const uint64_t& tdp_abs_time, uint8_t tdp_doy,
                                    bool tdp_valid, const TDF1CSDWFmt& tdf1csdw)
{
    tdp_valid_ = tdp_valid;

    // Do not update any values if tdp is not valid
    if (tdp_valid)
    {
        // This function should only be called by the TDP parser at
        // the time it is parsed. If so, the RTC stored in this
        // instance of Ch10Context is the TDP RTC. Re-assign it as such.
        // RTC is already in units of nanoseconds.
        tdp_rtc_ = rtc_;

        // Store the tdp absolute time.
        tdp_abs_time_ = tdp_abs_time;

        // Store the tdp doy.
        tdp_doy_ = tdp_doy;

        // Indicate that the TDP has been found.
        found_tdp_ = true;
    }

    tdf1csdw_vec_.push_back(tdf1csdw);
    tdp_abs_time_vec_.push_back(tdp_abs_time);
}

uint64_t& Ch10Context::CalculateAbsTimeFromRTCFormat(const uint64_t& current_rtc)
{
    temp_abs_time_ = tdp_abs_time_ + (current_rtc - tdp_rtc_);
    return temp_abs_time_;
}

uint64_t& Ch10Context::GetPacketAbsoluteTimeFromHeaderRTC()
{
    temp_abs_time_ = tdp_abs_time_ + (rtc_ - tdp_rtc_);
    return temp_abs_time_;
}

uint64_t& Ch10Context::CalculateIPTSAbsTime(const uint64_t& ipts_time)
{
    // Handle RTC time or other format indicated by time_format_. RTC time is indicated
    // by intrapkt_ts_src_ = 0.
    if (intrapkt_ts_src_ == 0)
    {
        return CalculateAbsTimeFromRTCFormat(ipts_time);
    }
    else
    {
        // If the time source is the packet secondary header,
        // the time calculated from the IPTS is already absolute time.
        // Simply return the input time.
        temp_abs_time_ = ipts_time;
        return temp_abs_time_;
    }
}

 uint64_t& Ch10Context::Calculate429WordAbsTime(const uint64_t& total_gap_time)
{
    // Handle RTC time or other format indicated by time_format_. RTC time is indicated
    // by intrapkt_ts_src_ = 0.
    if (intrapkt_ts_src_ == 0)
    {
        // modify the total_gap_time to be in units of ipts_time and add with abs time
        temp_abs_time_ = GetPacketAbsoluteTimeFromHeaderRTC() + (total_gap_time * (uint64_t)100);
        return temp_abs_time_;
    }
    else
    {
        // If the time source is the packet secondary header,
        // the time of packt_abs_time_ is already abs_time.
        // Simply add Gap Time (adjusted to nano seconds) to
        // packet_abs_time and return.
        temp_abs_time_ = packet_abs_time_ + (total_gap_time * (uint64_t)100);
        return temp_abs_time_;
    }
}

void Ch10Context::UpdateChannelIDToLRUAddressMaps(const uint32_t& chanid,
                                                  const MilStd1553F1DataHeaderCommWordFmt* const data_header)
{
    // Set pointers for command words 1 and 2.
    const MilStd1553F1DataHeaderCommWordOnlyFmt* comm_words =
        (const MilStd1553F1DataHeaderCommWordOnlyFmt*)data_header;

    if (data_header->RR)
    {
        chanid_remoteaddr1_map_[chanid].insert(data_header->remote_addr1);
        chanid_remoteaddr2_map_[chanid].insert(data_header->remote_addr2);
        chanid_commwords_map_[chanid].insert(
            (uint32_t(comm_words->comm_word2) << 16) + comm_words->comm_word1);
    }
    else
    {
        chanid_remoteaddr1_map_[chanid].insert(data_header->remote_addr1);
        if (data_header->tx1)
            chanid_commwords_map_[chanid].insert(uint32_t(comm_words->comm_word1) << 16);
        else
            chanid_commwords_map_[chanid].insert(comm_words->comm_word1);
    }
}

void Ch10Context::UpdateARINC429Maps(const uint32_t& chanid,
                                     const ARINC429F0MsgFmt* const data_header)
{
    chanid_labels_map_[chanid].insert(data_header->label);
    chanid_busnumbers_map_[chanid].insert(data_header->bus);
    chanid_busnumbers_labels_map_[chanid][data_header->bus].insert(data_header->label);
}

bool Ch10Context::CheckConfiguration(
    const std::unordered_map<Ch10PacketType, bool>& pkt_type_enabled_config,
    const std::map<Ch10PacketType, ManagedPath>& pkt_type_paths_config,
    std::map<Ch10PacketType, ManagedPath>& enabled_paths)
{
    // Loop over enabled map and check for presence of paths.
    using MapIt = std::unordered_map<Ch10PacketType, bool>::const_iterator;
    for (MapIt it = pkt_type_enabled_config.cbegin(); it != pkt_type_enabled_config.cend();
         ++it)
    {
        // If the packet type is TMATS or TDP, don't check for the path.
        if (!(it->first == Ch10PacketType::COMPUTER_GENERATED_DATA_F1 ||
              it->first == Ch10PacketType::TIME_DATA_F1))
        {
            // If the current type is enabled, then the path is relevant.
            if (it->second)
            {
                // If a key-value pair does not exist for the current type, return false.
                if (pkt_type_paths_config.count(it->first) == 0)
                {
                    SPDLOG_INFO(
                        "({:02d}) packet type {:d} is enabled and "
                        "a ManagedPath object does not exist in paths config map!",
                        thread_id, static_cast<uint8_t>(it->first));

                    // Clear the output map.
                    enabled_paths.clear();
                    return false;
                }

                // Otherwise insert the enabled type and the corresponding ManagedPath object.
                else
                {
                    enabled_paths[it->first] = pkt_type_paths_config.at(it->first);
                }
            }
        }
    }
    is_configured_ = true;
    return true;
}

bool Ch10Context::IsConfigured()
{
    return is_configured_;
}

int Ch10Context::InitializeFileWriters(const std::map<Ch10PacketType, ManagedPath>& enabled_paths)
{
    // Loop over enabled packet types and create, then submit to relevant parser,
    // a pointer to the file writer.
    int retcode = 0;
    using MapIt = std::map<Ch10PacketType, ManagedPath>::const_iterator;
    for (MapIt it = enabled_paths.cbegin(); it != enabled_paths.cend(); ++it)
    {
        SPDLOG_DEBUG("({:02d}) Enabling writer for {:s}",
                     thread_id, ch10packettype_to_string_map.at(it->first));

        switch (it->first)
        {
            case Ch10PacketType::MILSTD1553_F1:

                // Store the file writer status for this type as enabled.
                pkt_type_file_writers_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
                pkt_type_paths_enabled_map_[Ch10PacketType::MILSTD1553_F1] = it->second;

                // Create the writer object.
                milstd1553f1_pq_ctx_ = std::make_unique<ParquetContext>();
                milstd1553f1_pq_writer_ = std::make_unique<ParquetMilStd1553F1>(milstd1553f1_pq_ctx_.get());

                // Creating this publically accessible pointer is probably not the best
                // way to make the writer available, since it's now availalbe to everything.
                // Perhaps I should create a function for each parser (inherits from Ch10PacketComponent)
                // or maybe one in the base class in which to pass a const pointer that is stored
                // in the parser class. This breaks the paradigm that a file writer is very context
                // specific and should be held by Ch10Context. This will need careful thinking
                // and rework at some point.
                if ((retcode = milstd1553f1_pq_writer_->Initialize(it->second, thread_id)) != 0)
                    return retcode;
                milstd1553f1_pq_writer = milstd1553f1_pq_writer_.get();
                break;
            case Ch10PacketType::VIDEO_DATA_F0:

                pkt_type_file_writers_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = true;
                pkt_type_paths_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = it->second;

                // Create the writer object.
                videof0_pq_ctx_ = std::make_unique<ParquetContext>();
                videof0_pq_writer_ = std::make_unique<ParquetVideoDataF0>(videof0_pq_ctx_.get());
                if ((retcode = videof0_pq_writer_->Initialize(it->second, thread_id)) != 0)
                    return retcode;
                videof0_pq_writer = videof0_pq_writer_.get();
                break;
            case Ch10PacketType::ETHERNET_DATA_F0:

                pkt_type_file_writers_enabled_map_[Ch10PacketType::ETHERNET_DATA_F0] = true;
                pkt_type_paths_enabled_map_[Ch10PacketType::ETHERNET_DATA_F0] = it->second;
                ethernetf0_pq_ctx_ = std::make_unique<ParquetContext>();
                ethernetf0_pq_writer_ = std::make_unique<ParquetEthernetF0>(ethernetf0_pq_ctx_.get());
                if ((retcode = ethernetf0_pq_writer_->Initialize(it->second, thread_id)) != 0)
                    return retcode;
                ethernetf0_pq_writer = ethernetf0_pq_writer_.get();
                break;
            case Ch10PacketType::ARINC429_F0:

                // Store the file writer status for this type as enabled.
                pkt_type_file_writers_enabled_map_[Ch10PacketType::ARINC429_F0] = true;
                pkt_type_paths_enabled_map_[Ch10PacketType::ARINC429_F0] = it->second;

                // Create the writer object.
                arinc429f0_pq_ctx_ = std::make_unique<ParquetContext>();
                arinc429f0_pq_writer_ = std::make_unique<ParquetARINC429F0>(arinc429f0_pq_ctx_.get());

                // See note after the milstd1553f1_pq_writer_ defined above.
                // That thought applies here.
                if((retcode = arinc429f0_pq_writer_->Initialize(it->second, thread_id)) != 0)
                    return retcode;
                arinc429f0_pq_writer = arinc429f0_pq_writer_.get();
                break;
            default:
                SPDLOG_WARN("({:02d}) No writer defined for {:s}",
                            thread_id, ch10packettype_to_string_map.at(it->first));
                break;
        }
    }
    return 0;
}

void Ch10Context::CloseFileWriters() const
{
    using MapIt = std::unordered_map<Ch10PacketType, bool>::const_iterator;
    for (MapIt it = pkt_type_file_writers_enabled_map_.cbegin();
         it != pkt_type_file_writers_enabled_map_.cend(); ++it)
    {
        switch (it->first)
        {
            case Ch10PacketType::MILSTD1553_F1:
                if (pkt_type_file_writers_enabled_map_.at(Ch10PacketType::MILSTD1553_F1))
                    milstd1553f1_pq_ctx_->Close(thread_id_);
                break;
            case Ch10PacketType::VIDEO_DATA_F0:
                if (pkt_type_file_writers_enabled_map_.at(Ch10PacketType::VIDEO_DATA_F0))
                    videof0_pq_ctx_->Close(thread_id_);
                break;
            case Ch10PacketType::ETHERNET_DATA_F0:
                if (pkt_type_file_writers_enabled_map_.at(Ch10PacketType::ETHERNET_DATA_F0))
                    ethernetf0_pq_ctx_->Close(thread_id_);
                break;
            case Ch10PacketType::ARINC429_F0:
                if (pkt_type_file_writers_enabled_map_.at(Ch10PacketType::ARINC429_F0))
                    arinc429f0_pq_ctx_->Close(thread_id_);
                break;
            case Ch10PacketType::COMPUTER_GENERATED_DATA_F1:
                // No writer for this type
                break;
            case Ch10PacketType::TIME_DATA_F1:
                // No writer for this type
                break;
            case Ch10PacketType::NONE:
                // Error if this type.
                SPDLOG_ERROR("Ch10PacketType is {:s}. Not a valid type.",
                    ch10packettype_to_string_map.at(Ch10PacketType::NONE));
                break;
        }
    }
}

void Ch10Context::RecordMinVideoTimeStamp(const uint64_t& ts)
{
    if (chanid_minvideotimestamp_map_.count(channel_id_) == 0)
        chanid_minvideotimestamp_map_[channel_id_] = ts;
    else
    {
        if (ts < chanid_minvideotimestamp_map_.at(channel_id_))
            chanid_minvideotimestamp_map_[channel_id_] = ts;
    }
}

bool Ch10Context::RegisterUnhandledPacketType(const Ch10PacketType& pkt_type)
{
    if (registered_unhandled_packet_types_.count(pkt_type) == 0)
    {
        registered_unhandled_packet_types_.insert(pkt_type);
        return true;
    }

    return false;
}