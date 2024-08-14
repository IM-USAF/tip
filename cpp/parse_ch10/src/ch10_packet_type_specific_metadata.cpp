#include "ch10_packet_type_specific_metadata.h"

Ch10PacketTypeSpecificMetadataFunctions::Ch10PacketTypeSpecificMetadataFunctions() : it_()
{}


bool Ch10PacketTypeSpecificMetadata::RecordMilStd1553F1SpecificMetadata(
    std::vector<const Ch10Context*> context_vec,
    MDCategoryMap* runtime_metadata)
{
    return RecordMilStd1553F1SpecificMetadata(context_vec, runtime_metadata, &funcs_);
}

bool Ch10PacketTypeSpecificMetadata::RecordVideoDataF0SpecificMetadata(
    std::vector<const Ch10Context*> context_vec,
    MDCategoryMap* runtime_metadata)
{
    return RecordVideoDataF0SpecificMetadata(context_vec, runtime_metadata, &funcs_);
}

bool Ch10PacketTypeSpecificMetadata::RecordARINC429F0SpecificMetadata(
    std::vector<const Ch10Context*> context_vec,
    MDCategoryMap* runtime_metadata, const TMATSData* tmats)
{
    return RecordARINC429F0SpecificMetadata(context_vec, runtime_metadata,
        tmats, &funcs_);
}

bool Ch10PacketTypeSpecificMetadata::RecordMilStd1553F1SpecificMetadata(
    std::vector<const Ch10Context*> context_vec, MDCategoryMap* runtime_metadata,
    Ch10PacketTypeSpecificMetadataFunctions* func)
{
    // Obtain the tx and rx combined channel ID to LRU address map and
    // record it to the Yaml writer. First compile all the channel ID to
    // LRU address maps from the workers.
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr1_maps;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr2_maps;
    for (uint16_t worker_ind = 0; worker_ind < context_vec.size(); worker_ind++)
    {
        chanid_lruaddr1_maps.push_back(context_vec.at(worker_ind)->GetChannelIDToRemoteAddr1Map());
        chanid_lruaddr2_maps.push_back(context_vec.at(worker_ind)->GetChannelIDToRemoteAddr2Map());
    }
    std::map<uint32_t, std::set<uint16_t>> output_chanid_remoteaddr_map;
    if (!func->CombineChannelIDToLRUAddressesMetadata(output_chanid_remoteaddr_map,
                                                chanid_lruaddr1_maps, chanid_lruaddr2_maps))
        return false;
    runtime_metadata->SetArbitraryMappedValue("chanid_to_lru_addrs",
        output_chanid_remoteaddr_map);

    // Obtain the channel ID to command words set map.
    std::vector<std::map<uint32_t, std::set<uint32_t>>> chanid_commwords_maps;
    for (uint16_t worker_ind = 0; worker_ind < context_vec.size(); worker_ind++)
        chanid_commwords_maps.push_back(context_vec.at(worker_ind)->GetChannelIDToCommWordsMap());

    std::map<uint32_t, std::vector<std::vector<uint32_t>>> output_chanid_commwords_map;
    if (!func->CombineChannelIDToCommandWordsMetadata(output_chanid_commwords_map,
                                                chanid_commwords_maps))
        return false;
    runtime_metadata->SetArbitraryMappedValue("chanid_to_comm_words",
        output_chanid_commwords_map);

    return true;
}

bool Ch10PacketTypeSpecificMetadata::RecordVideoDataF0SpecificMetadata(
    std::vector<const Ch10Context*> context_vec, MDCategoryMap* runtime_metadata,
    Ch10PacketTypeSpecificMetadataFunctions* func)
{
     // Get the channel ID to minimum time stamp map.
    std::vector<std::map<uint16_t, uint64_t>> worker_chanid_to_mintimestamps_maps;
    for (uint16_t worker_ind = 0; worker_ind < context_vec.size(); worker_ind++)
    {
        worker_chanid_to_mintimestamps_maps.push_back(
            context_vec.at(worker_ind)->GetChannelIDToMinVideoTimestampMap());
    }
    std::map<uint16_t, uint64_t> output_min_timestamp_map;
    func->CreateChannelIDToMinVideoTimestampsMetadata(output_min_timestamp_map,
                                                worker_chanid_to_mintimestamps_maps);

    // Record the map in the Yaml writer and write the
    // total yaml text to file.
    runtime_metadata->SetArbitraryMappedValue("chanid_to_first_timestamp",
        output_min_timestamp_map);

    return true;
}

bool Ch10PacketTypeSpecificMetadata::RecordARINC429F0SpecificMetadata(
    std::vector<const Ch10Context*> context_vec, MDCategoryMap* runtime_metadata,
    const TMATSData* tmats, Ch10PacketTypeSpecificMetadataFunctions* func)
{
    // Obtain the channel ID to 429 label set map.
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_label_maps;
    for (uint16_t worker_ind = 0; worker_ind < context_vec.size(); worker_ind++)
        chanid_label_maps.push_back(context_vec.at(worker_ind)->GetChannelIDToLabelsMap());

    std::map<uint32_t, std::set<uint16_t>> output_chanid_label_map;
    if (!func->CombineChannelIDToLabelsMetadata(output_chanid_label_map,
                                                chanid_label_maps))
        return false;
    runtime_metadata->SetArbitraryMappedValue("chanid_to_labels",
        output_chanid_label_map);

    // Obtain the channel ID to ARINC message header bus number set map.
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_busnumber_maps;
    for (uint16_t worker_ind = 0; worker_ind < context_vec.size(); worker_ind++)
        chanid_busnumber_maps.push_back(context_vec.at(worker_ind)->GetChannelIDToBusNumbersMap());

    std::map<uint32_t, std::set<uint16_t>> output_chanid_busnumber_map;
    if (!func->CombineChannelIDToBusNumbersMetadata(output_chanid_busnumber_map,
                                                chanid_busnumber_maps))
        return false;
    runtime_metadata->SetArbitraryMappedValue("chanid_to_bus_numbers",
        output_chanid_busnumber_map);

    // Obtain Channel Id to subchannel id to labels maps
    std::vector<std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>> chanid_busnumbers_labels_maps;
    for (uint16_t worker_ind = 0; worker_ind < context_vec.size(); worker_ind++)
        chanid_busnumbers_labels_maps.push_back(context_vec.at(worker_ind)->GetChannelIDToBusNumbersToLabelsMap());

    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> output_chanid_busnumbers_labels_map;
    if (!func->CombineChannelIDToBusNumbersToLabelsMetadata(output_chanid_busnumbers_labels_map,
                                                chanid_busnumbers_labels_maps))
        return false;

    runtime_metadata->SetArbitraryMappedValue("chanid_to_bus_numbers_to_labels",
        output_chanid_busnumbers_labels_map);

    // Record ARINC429-specific TMATS data
    runtime_metadata->SetArbitraryMappedValue("tmats_chanid_to_429_format",
        tmats->GetChannelIDTo429Format());
    runtime_metadata->SetArbitraryMappedValue("tmats_chanid_to_429_subchans",
        tmats->GetChannelIDTo429Subchans());
    runtime_metadata->SetArbitraryMappedValue("tmats_chanid_to_429_subchan_and_name",
        tmats->GetChannelIDTo429SubchanAndName());

    return true;
}

void Ch10PacketTypeSpecificMetadataFunctions::CreateChannelIDToMinVideoTimestampsMetadata(
    std::map<uint16_t, uint64_t>& output_chanid_to_mintimestamp_map,
    const std::vector<std::map<uint16_t, uint64_t>>& chanid_mintimestamp_maps) const
{
    // Gather the maps from each worker and combine them into one,
    //keeping only the lowest time stamps for each channel ID.
    for (size_t i = 0; i < chanid_mintimestamp_maps.size(); i++)
    {
        std::map<uint16_t, uint64_t> temp_map = chanid_mintimestamp_maps.at(i);
        for (std::map<uint16_t, uint64_t>::const_iterator it = temp_map.begin();
             it != temp_map.end(); ++it)
        {
            if (output_chanid_to_mintimestamp_map.count(it->first) == 0)
                output_chanid_to_mintimestamp_map[it->first] = it->second;
            else if (it->second < output_chanid_to_mintimestamp_map[it->first])
                output_chanid_to_mintimestamp_map[it->first] = it->second;
        }
    }
}

bool Ch10PacketTypeSpecificMetadataFunctions::CombineChannelIDToLRUAddressesMetadata(
    std::map<uint32_t, std::set<uint16_t>>& output_chanid_lruaddr_map,
    const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr1_maps,
    const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr2_maps) const
{
    // Input vectors must have the same length.
    if (chanid_lruaddr1_maps.size() != chanid_lruaddr2_maps.size())
    {
        spdlog::get("pm_logger")->warn(
            "CombineChannelIDToLRUAddressesMetadata: "
            "Input vectors are not the same size, chanid_lruaddr1_maps ({:d}) "
            "chanid_lruaddr2_maps ({:d})",
            chanid_lruaddr1_maps.size(), chanid_lruaddr2_maps.size());
        return false;
    }

    // Collect and combine the channel ID to LRU address maps
    // assembled by each worker.
    std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map1;
    std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map2;
    for (size_t i = 0; i < chanid_lruaddr1_maps.size(); i++)
    {
        //workers[read_ind].append_chanid_remoteaddr_maps(chanid_remoteaddr_map1, chanid_remoteaddr_map2);
        chanid_remoteaddr_map1 = it_.CombineCompoundMapsToSet(
            chanid_remoteaddr_map1, chanid_lruaddr1_maps.at(i));
        chanid_remoteaddr_map2 = it_.CombineCompoundMapsToSet(
            chanid_remoteaddr_map2, chanid_lruaddr2_maps.at(i));
    }

    // Combine the tx and rx maps into a single map.
    output_chanid_lruaddr_map = it_.CombineCompoundMapsToSet(
        chanid_remoteaddr_map1, chanid_remoteaddr_map2);

    return true;
}

bool Ch10PacketTypeSpecificMetadataFunctions::CombineChannelIDToCommandWordsMetadata(
    std::map<uint32_t, std::vector<std::vector<uint32_t>>>& output_chanid_commwords_map,
    const std::vector<std::map<uint32_t, std::set<uint32_t>>>& chanid_commwords_maps) const
{
    // Collect maps into one.
    std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map;
    for (size_t i = 0; i < chanid_commwords_maps.size(); i++)
    {
        chanid_commwords_map = it_.CombineCompoundMapsToSet(chanid_commwords_map,
                                                            chanid_commwords_maps.at(i));
    }

    // Break compound command words each into a set of two command words,
    // a transmit and receive value.
    uint32_t mask_val = (1 << 16) - 1;
    for (std::map<uint32_t, std::set<uint32_t>>::const_iterator it = chanid_commwords_map.cbegin();
         it != chanid_commwords_map.cend(); ++it)
    {
        std::vector<std::vector<uint32_t>> temp_vec_of_vec;
        for (std::set<uint32_t>::const_iterator it2 = it->second.cbegin();
             it2 != it->second.cend(); ++it2)
        {
            // Vector needed here to retain order.
            std::vector<uint32_t> pair_vec = {*it2 >> 16, *it2 & mask_val};
            temp_vec_of_vec.push_back(pair_vec);
        }
        output_chanid_commwords_map[it->first] = temp_vec_of_vec;
    }
    return true;
}

bool Ch10PacketTypeSpecificMetadataFunctions::CombineChannelIDToLabelsMetadata(
    std::map<uint32_t, std::set<uint16_t>>& output_chanid_labels_map,
        const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_labels_maps) const
{
    // Collect maps into one.
    std::map<uint32_t, std::set<uint16_t>> chanid_labels_map;
    for (size_t i = 0; i < chanid_labels_maps.size(); i++)
    {
        chanid_labels_map = it_.CombineCompoundMapsToSet(chanid_labels_map,
                                                            chanid_labels_maps.at(i));
    }

    // iterate chanid_labels_map and add to output_chanid_labels_map
    output_chanid_labels_map = chanid_labels_map;

    return true;
}

bool Ch10PacketTypeSpecificMetadataFunctions::CombineChannelIDToBusNumbersMetadata(
    std::map<uint32_t, std::set<uint16_t>>&  output_chanid_busnumbers_map,
        const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_busnumbers_maps) const
{
    // Collect maps into one.
    std::map<uint32_t, std::set<uint16_t>> chanid_busnumbers_map;
    for (size_t i = 0; i < chanid_busnumbers_maps.size(); i++)
    {
        chanid_busnumbers_map = it_.CombineCompoundMapsToSet(chanid_busnumbers_map,
                                                            chanid_busnumbers_maps.at(i));
    }

    // iterate chanid_busnumbers_map and add to output_chanid_busnumbers_map
    output_chanid_busnumbers_map = chanid_busnumbers_map;

    return true;
}

bool Ch10PacketTypeSpecificMetadataFunctions::CombineChannelIDToBusNumbersToLabelsMetadata(
    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>& output_chanid_busnumbers_labels_map,
    const std::vector<std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>>& chanid_busnumbers_labels_maps) const
{
    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> output_map;

    // iterate over vector of nested maps
    for (size_t i = 0; i < chanid_busnumbers_labels_maps.size(); i++)
    {
        std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> temp_map = chanid_busnumbers_labels_maps.at(i);

        // iterate chanid and nested busnum+labels map
        std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>::iterator it;
        for (it = temp_map.begin(); it != temp_map.end();
             it++)
        {
            uint32_t chanid = it->first;

            //  iterate busnumber and set of labels
            std::map<uint32_t, std::set<uint16_t>>::iterator it2;
            for (it2 = it->second.begin(); it2 != it->second.end(); it2++)
            {
                uint32_t busnumber = it2->first;

                // if a key missing from output_map
                if (output_map.count(chanid) == 0 || output_map[chanid].count(busnumber) == 0)
                    output_map[chanid][busnumber] = it2->second;
                else
                    output_map[chanid][busnumber].insert(it2->second.begin(), it2->second.end());
            }
        }
    }

    output_chanid_busnumbers_labels_map = output_map;

    return true;
}

bool Ch10PacketTypeSpecificMetadataFunctions::PopulatePCMDataObject(
    const std::map<std::string, std::string>& code_to_vals, 
    Ch10PCMTMATSData& pcm_data)
{
    // Note: this function should only be used after verification
    // that required tmats attributes are present. This is accomplished
    // with TMATSParser::ParsePCMF1Data in TMATSData::Parse.
    ParseText pt;
    std::string code = "P-d\\DLN";
    if(it_.IsKeyInMap(code_to_vals, code))
        pcm_data.data_link_name_ = code_to_vals.at("P-d\\DLN");
    
    code = "P-d\\D2";
    if(it_.IsKeyInMap(code_to_vals, code))
    {
        double temp_val = 0.0;
        if(!pt.ConvertDouble(code_to_vals.at(code), temp_val))
        {
            spdlog::get("pm_logger")->error("PopulatePCMDataObject: Code "
            "{:s} for attribute {:s} failed to conver to float", 
            code, "bit_rate_");
            return false;
        }
        pcm_data.bit_rate_ = temp_val;
    }
    return true;
}



