#include "ch10_pcmf1_component.h"

Ch10Status Ch10PCMF1Component::Parse(const uint8_t*& data)
{
    // Parse the PCMF1 CSDW
    ParseElements(pcmf1_csdw_elem_vec_, data);


    return Ch10Status::OK;
}

Ch10Status Ch10PCMF1Component::ParseFrames(const uint8_t*& data)
{
    return Ch10Status::OK;
}