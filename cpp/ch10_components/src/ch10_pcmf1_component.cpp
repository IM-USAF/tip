#include "ch10_pcmf1_component.h"

Ch10Status Ch10PCMF1Component::Parse(const uint8_t*& data)
{
    if(ctx_->pcm_tmats_data_map.size() == 0)
    {
        SPDLOG_ERROR("Ch10PCMF1Component::Parse: Zero Ch10PCMTMATSData entries "
        "in map. Unable to proceed without context from TMATS.");
        return Ch10Status::PCMF1_ERROR;
    }
    if(ctx_->pcm_tmats_data_map.size() > 1)
    {
        SPDLOG_ERROR("Ch10PCMF1Component::Parse: >1 Ch10PCMTMATSData entries "
        "in map. Unable to proceed without context about which entry.");
        return Ch10Status::PCMF1_ERROR;
    }

    // Sanity check
    int first = ctx_->pcm_tmats_data_map.begin()->first;
    Ch10PCMTMATSData pcmdata = ctx_->pcm_tmats_data_map.at(first);
    SPDLOG_TRACE("Ch10PCMF1Component::Parse: Using TMATS index {:d} PCM "
        "data.", first);
    if(!pcmdata.CalculateMajorFrameLength(majframe_len_bits_))
    {
        SPDLOG_ERROR("Ch10PCMF1Component::Parse: Major frame length calculation "
            "does not agree. min_frames_in_maj_frame = {:d}, bits_in_min_frame = {:d}, "
            "min_frame_sync_pattern_len = {:d}, words_in_min_frame = {:d}, "
            "common_word_lenth = {:d}", pcmdata.min_frame_sync_pattern_len_, 
            pcmdata.bits_in_min_frame_, pcmdata.min_frame_sync_pattern_len_, 
            pcmdata.words_in_min_frame_, pcmdata.common_word_length_);
        return Ch10Status::PCMF1_ERROR;
    }

    // Parse the PCMF1 CSDW
    ParseElements(pcmf1_csdw_elem_vec_, data);

    // These modes are mutually exclusive -- really?
    // switch(static_cast<uint8_t>(*(*pcmf1_csdw_elem_.element) >> 18))
    // {
    //     case 0x01:
    //         // unpacked
    //         break;
    //     case 0x02: 
    //         // packed
    //         SPDLOG_ERROR("pcm packed mode!");
    //         break;
    //     case 0x03:
    //         //throughput
    //         break;
    //     case 0x04:
    //         // alignment
    //         break;
    // }

    // Sanity checks on parsed IPH data.
    if(!CheckFrameIndicator((*pcmf1_csdw_elem_.element)->mode_throughput, 
        (*pcmf1_csdw_elem_.element)->MI, (*pcmf1_csdw_elem_.element)->MA))
    {
        SPDLOG_ERROR("Ch10PCMF1Component::Parse: CheckFrameIndicator: "
            "throughput mode = {:d}, MI = {:d}, MA = {:d}", 
            (*pcmf1_csdw_elem_.element)->mode_throughput, 
            (*pcmf1_csdw_elem_.element)->MI,
            (*pcmf1_csdw_elem_.element)->MA);
        return Ch10Status::PCMF1_ERROR;
    }

    if((*pcmf1_csdw_elem_.element)->mode_throughput)
    {
        SPDLOG_WARN("Ch10PCMF1Component::Parse: Throughput mode not handled!");
        return Ch10Status::OK;
    }
    else if((*pcmf1_csdw_elem_.element)->mode_packed)
    {
        SPDLOG_ERROR("pcm packed mode!");
        return Ch10Status::OK;
    }
    else
    {
        // Must be unpacked if not the others?
        if(!(*pcmf1_csdw_elem_.element)->mode_unpacked)
        {
            SPDLOG_ERROR("Ch10PCMF1Component::Parse: Not packed, throughput, "
                "or unpacked mode?");
            return Ch10Status::PCMF1_ERROR;
        }
        return Ch10Status::OK;
    }

    // Determine count of minor frames and effective minor frame size, 
    // given word alignment implications. Not applicable in throughput 
    // mode. (need bits_in_min_frame, size of packet body, IPH bit, and
    // alignment mode 
    return Ch10Status::OK;
}

Ch10Status Ch10PCMF1Component::ParseFrames(const uint8_t*& data)
{
    // If IPH = 0 ==> no IPH. If IPH == 1, IPH (IPTS and IPDH) before
    // each minor frame.
    if((*pcmf1_csdw_elem_.element)->IPH)
    {

    }
    return Ch10Status::OK;
}

bool Ch10PCMF1Component::CheckFrameIndicator(const uint32_t& throughput, const uint32_t& MI, 
        const uint32_t& MA)
{
    if (throughput == 1)
        return true;
    else if(MI == 1 && MA == 1)
        return false;
    
    return true;
}

bool Ch10PCMF1Component::CalculateMinorFrameCount(
    const uint32_t& pkt_data_sz, const Ch10PCMTMATSData& tmats,
    const PCMF1CSDWFmt* hdr, 
    uint32_t& minor_frame_count, uint32_t& minor_frame_size)
{
    if(hdr->mode_throughput)
    {
        SPDLOG_ERROR("Ch10PCMF1Component::CalculateMinorFrameCount: "
            "throughput mode not valid.");
        return false;
    }
   return true; 
}


int Ch10PCMF1Component::GetPacketMinFrameSyncPatternBitCount(const PCMF1CSDWFmt* hdr, 
    const int& sync_pattern_len_bits)
{
    if(hdr->mode_unpacked == 1)
    {
        if(sync_pattern_len_bits < 17)
            return 16;
        else if(sync_pattern_len_bits < 33)
            return 32;
        else
        {
            if(hdr->mode_align == 0)
            {
                int word_count = (sync_pattern_len_bits + 15)/16;
                return 16*word_count;
            }
            else
            {
                int word_count = (sync_pattern_len_bits + 31)/32;
                return 32*word_count;
            }
        }
    }
    else if(hdr->mode_packed == 1)
        return sync_pattern_len_bits;
    else if(hdr->mode_throughput == 1)
        return sync_pattern_len_bits;
    else
        return -1;
}

int Ch10PCMF1Component::GetPacketMinFrameBitCount(const Ch10PCMTMATSData& tmats,
    const PCMF1CSDWFmt* hdr, const int& pkt_sync_pattern_bits)
{
    if(hdr->mode_unpacked)
    {
        // Does not handle common word size > 16 or any non-common 
        // word sizes.
        if(hdr->mode_align == 0)
        {
            return (tmats.words_in_min_frame_ - 1)*16
                + pkt_sync_pattern_bits;
        }
        else
        {
            int count = (tmats.words_in_min_frame_ - 1) * 16 + pkt_sync_pattern_bits;
            if(count % 32 != 0)
                count += 16;
            return count;
        }
    }
    else if(hdr->mode_packed)
    {
        // Does not handle any non-common word sizes
        if(hdr->mode_align == 0)
        {
            int count = (tmats.words_in_min_frame_ - 1) * tmats.common_word_length_
                + tmats.min_frame_sync_pattern_len_;
            if(count % 16 != 0)
            {
                int pad = 16 - (count % 16);
                return count + pad;
            }
            return count;
        }
        else
        {
            int count = (tmats.words_in_min_frame_ - 1) * tmats.common_word_length_
                + tmats.min_frame_sync_pattern_len_;
            if(count % 32 != 0)
            {
                int pad = 32 - (count % 32);
                return count + pad;
            }
            return count;
        }
    }
    else if(hdr->mode_throughput)
    {
        int count = (tmats.words_in_min_frame_ - 1)* tmats.common_word_length_
            + tmats.min_frame_sync_pattern_len_;
        if (count != tmats.bits_in_min_frame_)
        {
            SPDLOG_ERROR("Ch10PCMF1Component::GetPacketMinFrameBitCount: "
                "throughput mode calculated minor frame length ({:d}) does not "
                "equal value found in TMATS ({:d}). Note: unique word sizes is "
                "not currently handled. Calculation only uses single common word "
                "length.", count, tmats.bits_in_min_frame_);
            return -1;
        }
        return count;
    }

    return -1;
}
