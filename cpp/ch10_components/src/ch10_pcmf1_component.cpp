#include "ch10_pcmf1_component.h"

Ch10Status Ch10PCMF1Component::Parse(const uint8_t*& data)
{
    // Parse the PCMF1 CSDW
    ParseElements(pcmf1_csdw_elem_vec_, data);

    if((*pcmf1_csdw_elem_.element)->mode_throughput)
    {
        SPDLOG_WARN("Ch10PCMF1Component::Parse: Throughput mode not handled!");
        return Ch10Status::OK;
    }

    // Determine count of minor frames
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