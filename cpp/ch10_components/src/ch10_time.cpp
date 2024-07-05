#include "ch10_time.h"

Ch10Time::Ch10Time() : temp_ns_time_(0),
                       rtc_time_ptr_(nullptr),
                       binwt_time_ptr_(nullptr),
                       ieee_time_ptr_(nullptr),
                       ertc_time_ptr_(nullptr),
                       high_order_ns_(0),
                       low_order_ns_(0)
{
}

Ch10Time::~Ch10Time()
{
}

uint64_t& Ch10Time::CalculateRTCTimeFromComponents(const uint32_t& rtc1, const uint32_t& rtc2)
{
    temp_ns_time_ = ((uint64_t(rtc2) << 32) + uint64_t(rtc1)) * rtc_to_ns_;
    return temp_ns_time_;
}

Ch10Status Ch10Time::ParseSecondaryHeaderTime(const uint8_t*& data, uint8_t time_format,
                                              uint64_t& time_ns)
{
    switch (time_format)
    {
        case 0:
        {
            ParseBinaryWeightedTime(data, time_ns);
            SPDLOG_WARN("Binary Weighted Time is being used. "
                "Validate Ch10BinWtTimeStampFmt (ch10_time.h) struct and result!");
            break;
        }
        case 1:
        {
            ParseIEEE1588Time(data, time_ns);
            break;
        }
        case 2:
        {
            ParseERTCTime(data, time_ns);
            break;
        }
        default:
        {
            time_ns = 0;
            SPDLOG_ERROR("Invalid time_format = {:d}", time_format);
            return Ch10Status::INVALID_SECONDARY_HDR_FMT;
            break;
        }
    }

    // Advance data pointer past the time data block
    data += time_data_size_;
    return Ch10Status::OK;
}

void Ch10Time::ParseBinaryWeightedTime(const uint8_t*& data, uint64_t& time_ns)
{
    //binwt_time_ptr_ = (const Ch10BinWtTimeStampFmt *) data;
    binwt_time_ptr_ = reinterpret_cast<const Ch10BinWtTimeStampFmt*>(data);

    // Multiple by 655.36 second per bit, then 1e9 to get nanosecond.
    high_order_ns_ = static_cast<uint64_t>(binwt_time_ptr_->high_order * static_cast<double>(655.36)) * 1e9;

    // Multiple by 0.01 second per bit, then 1e9 to get nanosecond.
    low_order_ns_ = static_cast<uint64_t>(binwt_time_ptr_->low_order * static_cast<double>(0.01)) * 1e9;

    // Calculate nanoseconds from microsecond word.
    time_ns = static_cast<uint64_t>(binwt_time_ptr_->microsec) * 1e3;

    // Add other ns portions
    time_ns += high_order_ns_ + low_order_ns_;
}

void Ch10Time::ParseIEEE1588Time(const uint8_t*& data, uint64_t& time_ns)
{
    ieee_time_ptr_ = (const Ch10IEEE1588TimeStampFmt*)data;
    time_ns = uint64_t(ieee_time_ptr_->ns_word) + uint64_t(ieee_time_ptr_->sec_word) * 1e9;
}

void Ch10Time::ParseERTCTime(const uint8_t*& data, uint64_t& time_ns)
{
    ertc_time_ptr_ = (const Ch10ERTCTimeStampFmt*)data;
    time_ns = uint64_t(ertc_time_ptr_->lslw) + (uint64_t(ertc_time_ptr_->mslw) << 32);
}

void Ch10Time::ParseRTCTime(const uint8_t*& data, uint64_t& time_ns)
{
    rtc_time_ptr_ = (const Ch10RTCTimeStampFmt*)data;
    time_ns = ((uint64_t(rtc_time_ptr_->ts2_) << 32) + uint64_t(rtc_time_ptr_->ts1_)) * rtc_to_ns_;
}

Ch10Status Ch10Time::ParseIPTS(const uint8_t*& data, uint64_t& time_ns,
                               uint8_t intrapkt_ts_src, uint8_t time_fmt)
{
    if (intrapkt_ts_src == 0)
    {
        ParseRTCTime(data, time_ns);
        data += time_data_size_;
    }
    else if (intrapkt_ts_src == 1)
    {
        return ParseSecondaryHeaderTime(data, time_fmt, time_ns);
    }
    else
    {
        time_ns = 0;
        SPDLOG_WARN("Invalid intrapkt_ts_src = {:d}", intrapkt_ts_src);
        return Ch10Status::INVALID_INTRAPKT_TS_SRC;
    }

    return Ch10Status::OK;
}