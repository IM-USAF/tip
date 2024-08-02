#include "ch10_packet.h"

void Ch10Packet::SetCh10ComponentParsers(Ch10PacketHeaderComponent* header_comp,
        Ch10TMATSComponent* tmats_comp, Ch10TDPComponent* tdp_comp,
        Ch101553F1Component* milstd1553_comp, Ch10VideoF0Component* video_comp,
        Ch10EthernetF0Component* eth_comp, Ch10429F0Component* arinc429_comp)
{
    header_ = header_comp;
    tmats_ = tmats_comp;
    tdp_component_ = tdp_comp;
    milstd1553f1_component_ = milstd1553_comp;
    videof0_component_ = video_comp;
    ethernetf0_component_ = eth_comp;
    arinc429f0_component_ = arinc429_comp;
}

bool Ch10Packet::IsConfigured()
{

    // Pointers set by constructor
    if(bb_ == nullptr)
    {
        SPDLOG_CRITICAL("BinBuff pointer is nullptr");
        return false;
    }
    if(ctx_ == nullptr)
    {
        SPDLOG_CRITICAL("Ch10Context pointer is nullptr");
        return false;
    }
    if(ch10_time_ == nullptr)
    {
        SPDLOG_CRITICAL("Ch10Time pointer is nullptr");
        return false;
    }

    // Pointers set by SetCh10ComponentParsers
    if(header_ == nullptr)
    {
        SPDLOG_CRITICAL("Ch10PacketHeaderComponent pointer is nullptr");
        return false;
    }
    if(tmats_ == nullptr)
    {
        SPDLOG_CRITICAL("Ch10TMATSComponent pointer is nullptr");
        return false;
    }
    if(tdp_component_ == nullptr)
    {
        SPDLOG_CRITICAL("Ch10TDPComponent pointer is nullptr");
        return false;
    }
    if(milstd1553f1_component_ == nullptr)
    {
        SPDLOG_CRITICAL("Ch101553F1Component pointer is nullptr");
        return false;
    }
    if(videof0_component_ == nullptr)
    {
        SPDLOG_CRITICAL("Ch10VideoF0Component pointer is nullptr");
        return false;
    }
    if(ethernetf0_component_ == nullptr)
    {
        SPDLOG_CRITICAL("Ch10EthernetF0Component pointer is nullptr");
        return false;
    }
    if(arinc429f0_component_ == nullptr)
    {
        SPDLOG_CRITICAL("Ch10429F0Component pointer is nullptr");
        return false;
    }

    return true;
}

Ch10Status Ch10Packet::AdvanceBuffer(const uint64_t& byte_count)
{
    bb_response_ = bb_->AdvanceReadPos(byte_count);

    // If the buffer can't be advanced by the requested byte count, then
    // the current packet can't be parsed in it's entirety. In this case,
    // do not update the absolute position because parsing will need to
    // resume at the current absolute position later, in append mode.
    if (bb_response_ != 0)
        return Ch10Status::BUFFER_LIMITED;

    return Ch10Status::OK;
}

Ch10Status Ch10Packet::ManageHeaderParseStatus(const Ch10Status& status,
                                               const uint64_t& pkt_size)
{
    if (status != Ch10Status::OK)
    {
        // Ch10PacketHeaderComponent::Parse will return BAD_SYNC if
        // the parsed sync word is not correct. If this occurs
        // then the data need to be searched until a correct sync
        // word is found. Increment the buffer and absolute position
        // by a single byte and try again.
        // OR
        // If the header checksum fails, then a sync byte may have been found
        // accidentally and the packet size can't be trusted to be authentic
        // because it is parsed out of bits that may not be part of a real
        // packet header. Treat it as a bad sync and look for the next sync.
        if (status == Ch10Status::BAD_SYNC || status == Ch10Status::CHECKSUM_FALSE)
        {
            if (ctx_->thread_id == 0)
            {
                SPDLOG_DEBUG("({:02d}) status = {:s}", ctx_->thread_id,
                             Ch10StatusString(status));
            }
            status_ = AdvanceBuffer(1);

            // If the buffer can't be advanced, return this status.
            if (status_ == Ch10Status::BUFFER_LIMITED)
                return status_;

            // Advance the absolute position the same amount.
            ctx_->AdvanceAbsPos(1);

            // Otherwise indicate the bad sync status.
            return Ch10Status::BAD_SYNC;
        }

        // By default, skip the parsing of the packet and move the buffer to the
        // beginning of the next packet.
        else
        {
            SPDLOG_DEBUG("({:02d}) status = {:s}", ctx_->thread_id,
                         Ch10StatusString(status));
            status_ = AdvanceBuffer(pkt_size);

            // If the buffer can't be advanced, return this status.
            if (status_ == Ch10Status::BUFFER_LIMITED)
                return status_;

            // Advance the absolute position the same amount.
            ctx_->AdvanceAbsPos(pkt_size);

            // Otherwise return a status indicating that the packet body should be
            // skipped.
            return Ch10Status::PKT_TYPE_NO;
        }
    }
    return Ch10Status::OK;
}

Ch10Status Ch10Packet::ManageSecondaryHeaderParseStatus(const Ch10Status& status,
                                                        const uint64_t& pkt_size)
{
    if (status != Ch10Status::OK)
    {
        // Skip the entire packet if the secondary header does not parse correctly.
        // This may not be the best way to handle this situation. Amend if necessary.
        //if (status == Ch10Status::INVALID_SECONDARY_HDR_FMT)
        status_ = AdvanceBuffer(pkt_size);

        // Advance the absolute position the same amount.
        ctx_->AdvanceAbsPos(pkt_size);

        // If the buffer can't be advanced, return this status.
        if (status_ == Ch10Status::BUFFER_LIMITED)
            return status_;

        // Otherwise return a status indicating that the packet body should be
        // skipped.
        return Ch10Status::PKT_TYPE_NO;
    }
    return Ch10Status::OK;
}

Ch10Status Ch10Packet::ParseHeader(const uint64_t& abs_pos, bool& found_tmats)
{
    // Check if there are sufficient bytes in the buffer to
    // interpret the entire header, assuming initially that
    // that there is no secondary header.
    if (!bb_->BytesAvailable(header_->std_hdr_size_))
        return Ch10Status::BUFFER_LIMITED;

    // Set the data_ptr_ at the current position within the buffer.
    // This pointer will be incremented by Parse(...).
    data_ptr_ = bb_->Data();

    // Parse header and check authenticity
    status_ = header_->Parse(data_ptr_);
    const Ch10PacketHeaderFmt* const hdr = header_->GetHeader();
    temp_pkt_size_ = static_cast<uint64_t>(hdr->pkt_size);
    status_ = ManageHeaderParseStatus(status_, temp_pkt_size_);
    if (status_ != Ch10Status::OK)
        return status_;

    // Determine validity of TMATs presence
    Ch10Status tmats_status_;
    if (hdr->data_type == tmats_data_type_)
    {
        if((tmats_status_ = TmatsStatus(abs_pos, found_tmats, true)) != Ch10Status::OK)
            return tmats_status_;
    }
    else
    {
        if((tmats_status_ = TmatsStatus(abs_pos, found_tmats, false)) != Ch10Status::OK)
            return tmats_status_;
    }

    // Check if all of the bytes in current packet are available
    // in the buffer. There is no need to continue parsing the secondary
    // header or the packet body if all of the data are not present in
    // the buffer.
    if (!bb_->BytesAvailable(temp_pkt_size_))
        return Ch10Status::BUFFER_LIMITED;

    // Whether the header (possibly secondary header) have been parsed correctly,
    // move the absolute position and buffer position to the beginning of the next
    // ch10 header, which ought to occur after pkt_size bytes. Configuring these
    // values now instead of waiting until after the body has been parsed will allow
    // skipping to next header if other problems occur, for example by 'continue'ing
    // in the main loop if a certain status is returned. The various parsers needed
    // to parse the packet body will use data_ptr_ to track position.
    status_ = AdvanceBuffer(temp_pkt_size_);
    if (status_ == Ch10Status::BUFFER_LIMITED)
        return status_;

    // Configure context to prepare for use by the packet body parsers.
    status_ = ctx_->UpdateContext(ctx_->absolute_position + temp_pkt_size_,
                                  hdr,
                                  ch10_time_->CalculateRTCTimeFromComponents(hdr->rtc1,
                                                                            hdr->rtc2));
    if (status_ != Ch10Status::OK)
        return Ch10Status::PKT_TYPE_NO;

    // Check the data checksum now that it's known there are sufficient
    // bytes remaining to fully parse the packet. Do not proceed if the
    // data checksum is invalid.
    //
    // At this point data_ptr_ has been modified to point at the bytes immediately
    // after the header or header+secondary header.
    status_ = header_->VerifyDataChecksum(data_ptr_, hdr->checksum_existence,
                                         hdr->pkt_size, hdr->secondary_hdr);
    // If the checksum does not match, Skip this pkt.
    if (status_ == Ch10Status::CHECKSUM_FALSE)
        return Ch10Status::PKT_TYPE_NO;

    // Handle parsing of the secondary header. This is currently not
    // completed. Data parsed in this step are not utilized.
    status_ = header_->ParseSecondaryHeader(data_ptr_, secondary_hdr_time_ns_);
    status_ = ManageSecondaryHeaderParseStatus(status_, temp_pkt_size_);
    if (status_ != Ch10Status::OK)
        return status_;

    // Replace the relative time submitted in UpdateContext with the
    // secondary header time. This assumes that the secondary header
    // time is relative, which may not be true. Under this assumption,
    // absolute time determined from the TDP, relative time from the TDP
    // packet header and the current packet header time or IPTS are used
    // to calculate the absolute time. This calculation breaks down if
    // the secondary header time is not relative or it is relative and the
    // TDP header did not have a secondary header because a difference in
    // time resolution will exist between the non-secondary header TDP relative
    // time and secondary header relative time obtained in the current packet.
    if (secondary_hdr_time_ns_ != 0)
        ctx_->UpdateWithSecondaryHeaderTime(secondary_hdr_time_ns_);

    // Continue to parse depending on the context.
    return ctx_->ContinueWithPacketType(hdr->data_type);
}

Ch10Status Ch10Packet::ParseBody()
{
    // Note: to test this function, it will be easiest to add a function to
    // to BinBuff to set a buffer from memory. After this is complete, a
    // BinBuff object can be created at test time which points to the address
    // of a locally-created header object with a specific data type configured.
    // Then a Ch10Packet instance can be created with the instance of BinBuff as
    // an argument and then ParseHeader, then ParseBody can be called. The
    // pkt_type_ can be checked via current_pkt_type.
    pkt_type_ = Ch10PacketType::NONE;
    switch (header_->GetHeader()->data_type)
    {
        case static_cast<uint8_t>(Ch10PacketType::COMPUTER_GENERATED_DATA_F1):
            if (ctx_->IsPacketTypeEnabled(Ch10PacketType::COMPUTER_GENERATED_DATA_F1))
            {
                pkt_type_ = Ch10PacketType::COMPUTER_GENERATED_DATA_F1;
                tmats_->Parse(data_ptr_);
            }
            break;
        case static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1):
            if (ctx_->IsPacketTypeEnabled(Ch10PacketType::TIME_DATA_F1))
            {
                pkt_type_ = Ch10PacketType::TIME_DATA_F1;
                tdp_component_->Parse(data_ptr_);
            }
            break;
        case static_cast<uint8_t>(Ch10PacketType::MILSTD1553_F1):
            if (ctx_->IsPacketTypeEnabled(Ch10PacketType::MILSTD1553_F1))
            {
                pkt_type_ = Ch10PacketType::MILSTD1553_F1;
                milstd1553f1_component_->Parse(data_ptr_);
            }
            break;
        case static_cast<uint8_t>(Ch10PacketType::VIDEO_DATA_F0):
            if (ctx_->IsPacketTypeEnabled(Ch10PacketType::VIDEO_DATA_F0))
            {
                pkt_type_ = Ch10PacketType::VIDEO_DATA_F0;
                videof0_component_->Parse(data_ptr_);

                // Get the earliest subpacket absolute time that was parsed
                // out of the Ch10 video packet. If there are no intra-packet
                // headers, then all all of the subpacket times will be the same
                // and the first element in the subpacket_absolute_times vector
                // is still the earliest time stamp.
                //
                // Use the Ch10Context to keep a record of minimum timestamp
                // per channel ID.
                ctx_->RecordMinVideoTimeStamp(videof0_component_->subpacket_absolute_times.at(0));
            }
            break;
        case static_cast<uint8_t>(Ch10PacketType::ETHERNET_DATA_F0):
            if (ctx_->IsPacketTypeEnabled(Ch10PacketType::ETHERNET_DATA_F0))
            {
                pkt_type_ = Ch10PacketType::ETHERNET_DATA_F0;
                ethernetf0_component_->Parse(data_ptr_);
            }
            break;
        case static_cast<uint8_t>(Ch10PacketType::ARINC429_F0):
            if(ctx_->IsPacketTypeEnabled(Ch10PacketType::ARINC429_F0))
            {
                pkt_type_ = Ch10PacketType::ARINC429_F0;
                arinc429f0_component_->Parse(data_ptr_);
            }
            break;
        default:
            // If the packet type is configured (exists in the pkt_type_config_map)
            // and enabled (has a value of true) and does not have a parser, then
            // alert the user.
            Ch10PacketType pkt_type = static_cast<Ch10PacketType>(
                header_->GetHeader()->data_type);
            if (ctx_->RegisterUnhandledPacketType(pkt_type))
            {
                SPDLOG_WARN("({:02d}) No parser exists for {:s}", ctx_->thread_id,
                            ch10packettype_to_string_map.at(pkt_type));
            }
            break;
    }
    return Ch10Status::OK;
}

Ch10Status Ch10Packet::TmatsStatus(const uint64_t& abs_pos, 
    bool& found_tmats, bool curr_tmats)
{
    if(abs_pos > 0 && curr_tmats)
    {
        found_tmats = true;
        SPDLOG_ERROR("Ch10Packet: TMATS packer error--abs_pos == 0 "
            "(first worker) and TMATs packet encountered");
        return Ch10Status::TMATS_PKT_ERR;
    }
    else if(abs_pos == 0)
    {
        if(found_tmats)
        {
            if(curr_tmats)
                return Ch10Status::OK;
            else
            {
                SPDLOG_INFO("Ch10Packet: TMATS packet(s) found");
                return Ch10Status::TMATS_PKT;
            }
        }
        else 
        {
            if(curr_tmats)
            {
                found_tmats = true;
                return Ch10Status::OK;
            }
            else
            {
                SPDLOG_ERROR("Ch10Packet: TMATS packer error--TMATs packet "
                "not yet found and abs_pos == 0 (first worker)");
                return Ch10Status::TMATS_PKT_ERR;
            }
        }
    }
    return Ch10Status::OK;
}