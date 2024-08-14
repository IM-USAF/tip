#ifndef CH10_PACKET_TYPE_SPECIFIC_METADATA_H_
#define CH10_PACKET_TYPE_SPECIFIC_METADATA_H_

#include <string>
#include <map>
#include <vector>
#include <set>
#include "ch10_context.h"
#include "iterable_tools.h"
#include "parse_text.h"
#include "md_category_map.h"
#include "tmats_data.h"
#include "ch10_packet_type.h"
#include "ch10_pcm_tmats_data.h"
#include "spdlog/spdlog.h"


class Ch10PacketTypeSpecificMetadataFunctions
{
    private:
        IterableTools it_;

    public:
        Ch10PacketTypeSpecificMetadataFunctions();

        /*
        Combine channel ID to LRU address maps from vector of maps,
        where each map in the vector corresponds to a map retrieved from
        a worker. The output map is used in another function where it is
        recorded to metadata.

        Args:
            output_chanid_lruaddr_map	--> Output map, ch10 channel ID to set of
                                            all LRU addresses observed on the bus
                                            identifed by the given channel ID
            chanid_lruaddr1_maps		--> Vector of maps of channel ID to set of
                                            LRU addresses associated with the channel
                                            ID for the TX/RX command word
            chanid_lruaddr2_maps		--> Vector of maps of channel ID to set of
                                            LRU addresses associated with the channel
                                            ID for the RX command word for RT to RT
                                            messages. Must have size() equal to
                                            chanid_lruaddr1_maps.

        Return:
            True if no errors, false if errors occur and
            execution ought to stop.
        */
        virtual bool CombineChannelIDToLRUAddressesMetadata(
            std::map<uint32_t, std::set<uint16_t>>& output_chanid_lruaddr_map,
            const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr1_maps,
            const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr2_maps) const;

        /*
        Combine channel ID to command words maps from a vector of maps,
        where each map in the vector corresponds to a map of observed channel ID
        to command words retrieved from a worker. The output map is used in
        another function where it is recorded to metadata.

        Args:
        output_chanid_commwords_map	--> Output map, ch10 channel ID to a vector
                                        of vectors. Each final vector must have size()
                                        equal to 2, where the first entry is TX/RX
                                        command word and second entry is the RX
                                        command word in the case of RT to RT type
                                        messages. So each channel ID is mapped to a
                                        vector of command words that were observed
                                        on the bus during parsing. The bus is not
                                        known at this time, only the channel ID
                                        which represents a specific bus.
        chanid_commwords_maps		--> Vector of maps of channel ID to set of
                                        uint32_t value which is: commword1 << 16
                                        + commword2, where commword1 will be placed
                                        in the first position of the size-2 vector
                                        mentioned in the argument above, and
                                        commword2 will be placed in the second
                                        position.

        Return:
            True if no errors, false if errors occur and
            execution ought to stop.
        */
        virtual bool CombineChannelIDToCommandWordsMetadata(
            std::map<uint32_t, std::vector<std::vector<uint32_t>>>& output_chanid_commwords_map,
            const std::vector<std::map<uint32_t, std::set<uint32_t>>>& chanid_commwords_maps) const;

        /*
        Combine channel ID to 429 labels maps from a vector of maps,
        where each map in the vector corresponds to a map of observed channel ID
        to 429 labels retrieved from a worker. The output map is used in
        another function where it is recorded to metadata.

        Args:
        output_chanid_labels_map	--> Output map, ch10 channel ID to set of
                                        all 429 Labels observed on the bus
                                        identifed by the given channel ID.
        chanid_labels_maps  		--> Vector of maps of channel ID to set of ARINC
                                        429 labels associated with the channel ID.

        Return:
            True if no errors, false if errors occur and
            execution ought to stop.
        */
        virtual bool CombineChannelIDToLabelsMetadata(
            std::map<uint32_t, std::set<uint16_t>>& output_chanid_labels_map,
            const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_labels_maps) const;

        /*
        Combine channel ID to 429 IPDH Bus Numbers maps from a vector of maps,
        where each map in the vector corresponds to a map of observed channel ID
        to 429 IPDH Bus Numbers retrieved from a worker. The output map is used in
        another function where it is recorded to metadata.

        Args:
        output_chanid_busnumbers_map	--> Output map,ch10 channel ID to set of
                                        all IPDH Bus Numbers observed on the bus
                                        identifed by the given channel ID.
        chanid_busnumbers_maps		--> Vector of maps of channel ID to set of ARINC
                                        429 IPDH Bus Numbers associated with the channel
                                        ID.

        Return:
            True if no errors, false if errors occur and
            execution ought to stop.
        */
        virtual bool CombineChannelIDToBusNumbersMetadata(
            std::map<uint32_t, std::set<uint16_t>>&  output_chanid_busnumbers_map,
            const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_busnumbers_maps) const;

        /*
        Combine the channel ID to minimum timestamps map from each worker, already
        compiled as a vector maps, into the channel ID to absolute minimum timestamps
        map.

        output_chanid_to_mintimestamp_map	--> Output map, channel ID to minimum
                                                video packet timestamp for the channel
                                                ID. Timestamps in units of nanoseconds
                                                since the epoch (Unit time).
        chanid_mintimestamp_maps			--> Vector of maps of channel ID to minimum
                                                video packet time as observed by each
                                                worker.

        */
        virtual void CreateChannelIDToMinVideoTimestampsMetadata(
            std::map<uint16_t, uint64_t>& output_chanid_to_mintimestamp_map,
            const std::vector<std::map<uint16_t, uint64_t>>& chanid_mintimestamp_maps) const;

        /*
        Combine channel ID to 429 IPDH Bus Numbers to 429 Word label maps from a vector of maps,
        where each map in the vector corresponds to a map of observed channel ID
        to 429 IPDH Bus Numbers to labels retrieved from a worker. The output map is used in
        another function where it is recorded to metadata.

        Args:
        output_chanid_busnumbers_labels_map	--> Output map. Maps ch10 channel ID to set of all IPDH
                                                 Bus Numbers to labels observed on the bus.
        chanid_busnumbers_labels_maps		--> Vector of maps mapping ch10 channel ID to set of all IPDH
                                                 Bus Numbers to 429 word labels observed on the bus.

        Return:
            True if no errors, false if errors occur and
            execution ought to stop.
        */
        virtual bool CombineChannelIDToBusNumbersToLabelsMetadata(
            std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>&
                                                            output_chanid_busnumbers_labels_map,
            const std::vector<std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>>&
                                                            chanid_busnumbers_labels_maps) const;
                                                        
        /*
        Assign and cast values from a map of tmats codes to string values to 
        a Ch10PCMTMATSData object.

        Args:
            code_to_vals    --> string:string map of codes to values
            pcm_data        --> Reference to CH10PCMTMATSData object

        Return:
            False if a value can't be casted to the correct type, 
            otherwise true.

        */
        bool PopulatePCMDataObject(const std::map<std::string, std::string>& code_to_vals, 
            Ch10PCMTMATSData& pcm_data);

};

class Ch10PacketTypeSpecificMetadata
{
    private:
        Ch10PacketTypeSpecificMetadataFunctions funcs_;

    public:
        Ch10PacketTypeSpecificMetadata() : funcs_()
        {}

        bool RecordMilStd1553F1SpecificMetadata(std::vector<const Ch10Context*> context_vec,
            MDCategoryMap* runtime_metadata, Ch10PacketTypeSpecificMetadataFunctions* func);

        bool RecordVideoDataF0SpecificMetadata(std::vector<const Ch10Context*> context_vec,
            MDCategoryMap* runtime_metadata, Ch10PacketTypeSpecificMetadataFunctions* func);

        bool RecordARINC429F0SpecificMetadata(std::vector<const Ch10Context*> context_vec,
            MDCategoryMap* runtime_metadata, const TMATSData* tmats,
            Ch10PacketTypeSpecificMetadataFunctions* func);

        virtual bool RecordMilStd1553F1SpecificMetadata(std::vector<const Ch10Context*> context_vec,
            MDCategoryMap* runtime_metadata);

        virtual bool RecordVideoDataF0SpecificMetadata(std::vector<const Ch10Context*> context_vec,
            MDCategoryMap* runtime_metadata);

        virtual bool RecordARINC429F0SpecificMetadata(std::vector<const Ch10Context*> context_vec,
            MDCategoryMap* runtime_metadata, const TMATSData* tmats);


};



#endif  // CH10_PACKET_TYPE_SPECIFIC_METADATA_H_