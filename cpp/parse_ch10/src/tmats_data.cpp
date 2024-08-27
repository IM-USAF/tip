#include "tmats_data.h"

TMATSData::TMATSData() : chanid_to_source_map(chanid_to_source_map_), 
    chanid_to_type_map(chanid_to_type_map_),
    chanid_to_429_format(chanid_to_429_format_),
    chanid_to_429_subchans(chanid_to_429_subchans_),
    chanid_to_429_subchan_and_name(chanid_to_429_subchan_and_name_),
    // pcm_index_to_code_and_values(pcm_index_to_code_and_values_)
    tmats_pcm_data_map_(), tmats_pcm_data_map(tmats_pcm_data_map_),
    pt_()
{}

const std::map<Ch10PacketType, std::string> TMATSData::TMATS_channel_data_type_map_ = {
    {Ch10PacketType::MILSTD1553_F1, "1553IN"},
    {Ch10PacketType::ARINC429_F0, "429IN"},
    {Ch10PacketType::VIDEO_DATA_F0, "VIDIN"},
    {Ch10PacketType::ETHERNET_DATA_F0, "ETHIN"},
    {Ch10PacketType::PCM_F1, "PCMIN"}
};

bool TMATSData::Parse(const std::string& tmats_data, 
    const std::set<Ch10PacketType>& parsed_pkt_types)
{
    TMATSParser parser(tmats_data);

    // General 
    RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\DSI-n", chanid_to_source_map_), "chanid_to_source");
    RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n", chanid_to_type_map_), "chanid_to_type");

    // ARINC 429
    if(parsed_pkt_types.count(Ch10PacketType::ARINC429_F0) == 1)
    {
        RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\ABTF-n", chanid_to_429_format_), "chanid_to_429_format");
        RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\ASN-n-m", chanid_to_429_subchans_), "chanid_to_429_subchans");

        cmapmap chanid_to_429index_and_subchan;
        cmapmap chanid_to_429index_and_name;
        RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\ASN-n-m", chanid_to_429index_and_subchan), "chanid_to_429index_and_subchan");
        RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\ANM-n-m", chanid_to_429index_and_name), "chanid_to_429index_and_name");
    
        CombineMaps(chanid_to_429index_and_subchan, chanid_to_429index_and_name,
            chanid_to_429_subchan_and_name_);
    }

    // PCM F1
    if(parsed_pkt_types.count(Ch10PacketType::PCM_F1) == 1)
    {
        if(!ParsePCMAttributes(parser, tmats_pcm_data_map_))
            return false;
    }
    return true;
}


void TMATSData::CombineMaps(const cmapmap& map1, const cmapmap& map2, 
    cmapmap& outmap) const
{
    for(cmapmap::const_iterator it1 = map1.cbegin(); it1 != map1.cend(); ++it1)
    {
        if(map2.find(it1->first) != map2.cend())
        {
            cmap temp_map;
            cmap map1sub = it1->second;
            cmap map2sub = map2.at(it1->first);
            for(cmap::const_iterator it2 = map1sub.cbegin(); it2 != map1sub.cend(); ++it2) 
            {
                if(map2sub.find(it2->first) != map2sub.cend())
                {
                    temp_map[it2->second] = map2sub.at(it2->first);
                }
            }

            outmap[it1->first] = temp_map;
        }
    }
}

bool TMATSData::FilterTMATSType(const cmap& type_map, Ch10PacketType type_enum,
        cmap& filtered_map) const
{
    if(TMATS_channel_data_type_map_.count(type_enum) == 0)
    {
        SPDLOG_ERROR("Type \"{:s}\" not in TMATS_channel_data_type_map_",
            ch10packettype_to_string_map.at(type_enum));
        return false;
    }
    std::string type_string = TMATS_channel_data_type_map_.at(type_enum);

    for(cmap::const_iterator it = type_map.cbegin(); it != type_map.cend(); ++it)
    {
        if(it->second == type_string)
        {
            filtered_map[it->first] = it->second;
        }
    }
    return true;
}

cmap TMATSData::FilterByChannelIDToType(const cmap& type_map, const cmap& input_map) const
{
    cmap filtered_map;
    IterableTools iter;
    std::set<std::string> type_map_channel_ids = iter.VecToSet(iter.GetKeys(type_map));

    for(std::set<std::string>::const_iterator it = type_map_channel_ids.cbegin(); 
        it != type_map_channel_ids.cend(); ++it)
    {
        if(iter.IsKeyInMap(input_map, *it))
            filtered_map[*it] = input_map.at(*it);
    }

    return filtered_map;
}

bool TMATSData::PopulatePCMDataObject(const cmap& code_to_vals, 
    Ch10PCMTMATSData& pcm_data)
{
    // Note: this function should only be used after verification
    // that required tmats attributes are present. This is accomplished
    // with TMATSParser::ParsePCMF1Data in TMATSData::Parse. This 
    // function only checks if present tmats attributes can be casted
    // to the corresponding type and assigned to the corresponding
    // member variable of Ch10PCMTMATSDATA. 
    for(std::map<std::string, std::string>::const_iterator it = code_to_vals.cbegin();
        it != code_to_vals.cend(); ++it)
    {
        if(pcm_data.code_to_str_vals_map_.count(it->first) == 1)
        {
            POPFAIL(SetPCMDataValue(it->second, pcm_data.code_to_str_vals_map_.at(it->first)),
                it->second.c_str(), it->first.c_str())
        }
        else if(pcm_data.code_to_float_vals_map_.count(it->first) == 1)
        {
            POPFAIL(SetPCMDataValue(it->second, pcm_data.code_to_float_vals_map_.at(it->first)),
                it->second.c_str(), it->first.c_str())
        }
        else if(pcm_data.code_to_int_vals_map_.count(it->first) == 1)
        {
            POPFAIL(SetPCMDataValue(it->second, pcm_data.code_to_int_vals_map_.at(it->first)),
                it->second.c_str(), it->first.c_str())
        }
    }
    return true;
}


bool TMATSData::ParsePCMAttributes(TMATSParser& parser, pcmdata_map& pcmdata)
{
    if(!parser.ParsePCMF1Data(pcm_index_to_code_and_values_, 
        Ch10PCMTMATSData::pcm_req_attrs_, Ch10PCMTMATSData::pcm_opt_attrs_))
        return false;

    // Check presence of required attributes, cast to appropriate type,
    // and configure Ch10PCMTMATSData object
    for (unilateral_map::const_iterator it = pcm_index_to_code_and_values_.cbegin();
        it != pcm_index_to_code_and_values_.cend(); ++it)
    {
        Ch10PCMTMATSData temppcmdata;
        spdlog::get("pm_logger")->debug("ParsePCMAttributes: "
        "PopulatePCMDataObject for PCM TMATs index {:d}", it->first);
        if(!PopulatePCMDataObject(it->second, temppcmdata))
        {
            spdlog::get("pm_logger")->error("ParsePCMAttributes: "
            "PopulatePCMDataObject failed for PCM TMATs index {:d} due "
            "to missing required "
            "attributes or casting error", it->first);
            return false;
        }
        pcmdata[it->first] = temppcmdata;
    }
    return true;
}