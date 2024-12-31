
#include "ch10_packet_header_component.h"

Ch10Status Ch10PacketHeaderComponent::Parse(const uint8_t*& data)
{
    // Parse the standard header portion.
    ParseElements(std_elems_vec_, data);

    // If the sync pattern is not correct, then this is an invalid header.
    if ((*std_hdr_elem_.element)->sync != sync_)
        return Ch10Status::BAD_SYNC;

    // Calculate and compare checksum.
    status_ = VerifyHeaderChecksum((const uint8_t*)(*std_hdr_elem_.element),
                                   (*std_hdr_elem_.element)->checksum);
    if (status_ != Ch10Status::CHECKSUM_TRUE)
    {
        return status_;
    }

    return Ch10Status::OK;
}

void Ch10PacketHeaderComponent::ParseTimeStampNS(const uint8_t*& data, uint64_t& time_ns)
{
}

Ch10Status Ch10PacketHeaderComponent::ParseSecondaryHeader(const uint8_t*& data, uint64_t& time_ns)
{
    // Only parse if the secondary header option is enabled.
    if ((*std_hdr_elem_.element)->secondary_hdr == 1)
    {
        // Parse the time portion of the secondary header only.
        status_ = ch10_time_.ParseSecondaryHeaderTime(data,
                                                      (*std_hdr_elem_.element)->time_format, time_ns);

        // If the status is not OK, return the failed status code.
        if (status_ != Ch10Status::OK)
        {
            time_ns = 0;
            return status_;
        }

        // Calculate the secondary header checksum value. First
        // parse the position in memory that is located immediately
        // after the time data, which ought to be the current position of
        // the data pointer after the call to ParseSecondaryHeaderTime.
        // Then verify the checksum value by passing a pointer that is
        // time_data_size_ bytes prior.
        ParseElements(secondary_checksum_elems_vec_, data);
        status_ = VerifySecondaryHeaderChecksum(data - ch10_time_.time_data_size_,
                                                (*secondary_checksum_elem_.element)->checksum);
        if (status_ != Ch10Status::CHECKSUM_TRUE)
        {
            time_ns = 0;
            return status_;
        }
    }
    return Ch10Status::OK;
}

Ch10Status Ch10PacketHeaderComponent::VerifyHeaderChecksum(const uint8_t* pkt_data,
                                                           const uint32_t& checksum_value)
{
    checksum_unit_count_ = header_checksum_byte_count_ / 2;
    checksum_data_ptr16_ = (const uint16_t*)pkt_data;
    checksum_value16_ = 0;
    for (uint8_t i = 0; i < checksum_unit_count_; i++)
        checksum_value16_ += checksum_data_ptr16_[i];

    if (checksum_value16_ == checksum_value)
        return Ch10Status::CHECKSUM_TRUE;

    SPDLOG_WARN("({:02d}) Header checksum fail", ctx_->thread_id);
    return Ch10Status::CHECKSUM_FALSE;
}

Ch10Status Ch10PacketHeaderComponent::VerifySecondaryHeaderChecksum(const uint8_t* pkt_data,
                                                                    const uint16_t& checksum_value)
{
    checksum_data_ptr16_ = (const uint16_t*)pkt_data;
    checksum_value16_ = 0;
    for (uint8_t i = 0; i < 4; i++)
        checksum_value16_ += checksum_data_ptr16_[i];

    if (checksum_value16_ == checksum_value)
        return Ch10Status::CHECKSUM_TRUE;

    SPDLOG_WARN(
        "({:02d}) Secondary header checksum fail: calculated value = {:d}, "
        "expected value = {:d}",
        ctx_->thread_id, checksum_value16_, checksum_value);
    return Ch10Status::CHECKSUM_FALSE;
}

Ch10Status Ch10PacketHeaderComponent::VerifyDataChecksum(const uint8_t* body_data,
                                                         const uint32_t& checksum_existence, const uint32_t& pkt_size,
                                                         const uint32_t& secondary_hdr)
{
    // Calculate the total count of bytes in the data checksum.
    uint32_t checksum_byte_count = 0;
    if (secondary_hdr)
        checksum_byte_count = pkt_size - std_hdr_size_ - secondary_hdr_size_;
    else
        checksum_byte_count = pkt_size - std_hdr_size_;

    // 16-bit data checksum
    if (checksum_existence == 2)
    {
        // Adjust total byte count by the last few bytes in the footer
        // which are occupied by the value of the data checksum.
        checksum_byte_count -= 2;

        // Get the last 2 bytes from the packet and interpret as
        // the recorded checksum value.
        checksum_data_ptr16_ = (const uint16_t*)(body_data + checksum_byte_count);
        uint16_t checksum_value = *checksum_data_ptr16_;

        checksum_unit_count_ = checksum_byte_count / 2;
        checksum_data_ptr16_ = (const uint16_t*)body_data;
        checksum_value16_ = 0;
        for (uint32_t i = 0; i < checksum_unit_count_; i++)
        {
            checksum_value16_ += checksum_data_ptr16_[i];
            //printf("summing %hu\n", checksum_data_ptr16_[i]);
        }
        /*printf("Ch10PacketHeaderComponent::VerifyDataChecksum(): checksum_value16_ = %hu\n",
            checksum_value16_);
        printf("expected value = %hu\n", checksum_value);*/
        if (checksum_value16_ == checksum_value)
            return Ch10Status::CHECKSUM_TRUE;
    }
    // No data checksum present.
    else if (checksum_existence == 0)
        return Ch10Status::CHECKSUM_NOT_PRESENT;
    // 8-bit data checksum
    else if (checksum_existence == 1)
    {
        checksum_byte_count -= 1;

        checksum_data_ptr8_ = (const uint8_t*)(body_data + checksum_byte_count);
        uint8_t checksum_value = *checksum_data_ptr8_;

        checksum_unit_count_ = checksum_byte_count;
        checksum_data_ptr8_ = body_data;
        checksum_value8_ = 0;
        // printf("summming values:\n");
        for (uint32_t i = 0; i < checksum_unit_count_; i++)
        {
            checksum_value8_ += checksum_data_ptr8_[i];
            // printf("%hhu, ", checksum_data_ptr8_[i]);
        }
        // printf("value at i+1 = %hhu\n", checksum_data_ptr8_[checksum_unit_count_]);
        // printf("\nCh10PacketHeaderComponent::VerifyDataChecksum(): checksum_value8_ = %hhu\n",
        //     checksum_value8_);
        // printf("expected value = %hhu\n", checksum_value);
        if (checksum_value8_ == checksum_value)
            return Ch10Status::CHECKSUM_TRUE;
    }
    // 32-bit data checksum
    else if (checksum_existence == 3)
    {
        checksum_byte_count -= 4;

        checksum_data_ptr32_ = (const uint32_t*)(body_data + checksum_byte_count);
        uint32_t checksum_value = *checksum_data_ptr32_;

        checksum_unit_count_ = checksum_byte_count / 4;
        checksum_data_ptr32_ = (const uint32_t*)body_data;
        checksum_value32_ = 0;
        for (uint32_t i = 0; i < checksum_unit_count_; i++)
            checksum_value32_ += checksum_data_ptr32_[i];

        if (checksum_value32_ == checksum_value)
            return Ch10Status::CHECKSUM_TRUE;
    }

    SPDLOG_WARN("({:02d}) Data checksum fail", ctx_->thread_id);
    return Ch10Status::CHECKSUM_FALSE;
}