#ifndef CH10_PCM_TMATS_DATA_H_
#define CH10_PCM_TMATS_DATA_H_

#include <string>
#include <map>
#include <set>

class Ch10PCMTMATSData
{
    public:
        // See IRI6 106 Chapter 9, "Telemetry Attributes Transfer Standard" (TMATS)
        // for definitions of these PCM TMATS values.
        std::string data_link_name_;
        std::string pcm_code_; 
        std::string encrypted_;
        float bit_rate_;
        std::string polarity_;
        std::string auto_pol_correction_;
        std::string data_direction_;
        std::string data_randomized_;
        std::string randomizer_type_;
        std::string type_format_;
        int common_word_length_;
        std::string word_transfer_order_;
        std::string parity_;
        std::string parity_transfer_order_;
        std::string crc_;
        int crc_check_word_starting_bit_;
        int crc_data_start_bit_;
        int crc_data_number_of_bits_;
        int min_frames_in_maj_frame_;
        int words_in_min_frame_;
        int bits_in_min_frame_;
        std::string sync_type_;
        int min_frame_sync_pattern_len_;
        std::string min_frame_sync_pattern_bitdef_;

        static const std::set<std::string> pcm_req_attrs_;
        static const std::set<std::string> pcm_opt_attrs_;

        const std::map<std::string, int*> code_to_int_vals_map_;
        const std::map<std::string, float*> code_to_float_vals_map_;
        const std::map<std::string, std::string*> code_to_str_vals_map_;

        static const std::string null_indicator_;

        Ch10PCMTMATSData() : common_word_length_(-1), word_transfer_order_(null_indicator_),
            parity_(null_indicator_), crc_(null_indicator_), crc_check_word_starting_bit_(-1), 
            crc_data_start_bit_(-1), crc_data_number_of_bits_(-1), 
            min_frames_in_maj_frame_(-1), words_in_min_frame_(-1),
            bits_in_min_frame_(-1), data_link_name_(null_indicator_), 
            pcm_code_(null_indicator_), bit_rate_(-1), encrypted_(null_indicator_),
            polarity_(null_indicator_), auto_pol_correction_(null_indicator_),
            data_direction_(null_indicator_), data_randomized_(null_indicator_),
            randomizer_type_(null_indicator_), type_format_(null_indicator_),
            parity_transfer_order_(null_indicator_), sync_type_(null_indicator_),
            min_frame_sync_pattern_len_(-1), min_frame_sync_pattern_bitdef_(null_indicator_), 
            code_to_int_vals_map_{
                {"P-d\\F1", &common_word_length_},
                {"P-d\\CRCCB", &crc_check_word_starting_bit_},
                {"P-d\\CRCDB", &crc_data_start_bit_},
                {"P-d\\CRCDN", &crc_data_number_of_bits_},
                {"P-d\\MF\\N", &min_frames_in_maj_frame_},
                {"P-d\\MF1", &words_in_min_frame_},
                {"P-d\\MF2", &bits_in_min_frame_},
                {"P-d\\MF4", &min_frame_sync_pattern_len_},
            },
            code_to_float_vals_map_{
                {"P-d\\D2", &bit_rate_}
            },
            code_to_str_vals_map_{
                {"P-d\\TF", &type_format_},
                {"P-d\\DLN", &data_link_name_},
                {"P-d\\D1", &pcm_code_},
                {"P-d\\D3", &encrypted_},
                {"P-d\\D4", &polarity_},
                {"P-d\\D5", &auto_pol_correction_}, 
                {"P-d\\D6", &data_direction_},
                {"P-d\\D7", &data_randomized_},
                {"P-d\\D8", &randomizer_type_},
                {"P-d\\F2", &word_transfer_order_},
                {"P-d\\F3", &parity_},
                {"P-d\\F4", &parity_transfer_order_},
                {"P-d\\CRC", &crc_},
                {"P-d\\MF3", &sync_type_},
                {"P-d\\MF5", &min_frame_sync_pattern_bitdef_},
            }
        {}
        Ch10PCMTMATSData& operator=(const Ch10PCMTMATSData&);
        bool operator==(const Ch10PCMTMATSData&) const;

        /*
        Calculate major frame length using two methods and compare
        results. 

        Args:
            majframe_len    --> major frame length variable to be populated

        Return:
            True if major frame length calculated values agree. False
            otherwise.  
        */
        bool CalculateMajorFrameLength(int& majframe_len);
};

#endif  // CH10_PCM_TMATS_DATA_H_
