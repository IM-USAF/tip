#ifndef CH10_PCM_TMATS_DATA_H_
#define CH10_PCM_TMATS_DATA_H_

#include <string>

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
            parity_transfer_order_(null_indicator_), sync_type_(null_indicator_)
        {}
};

#endif  // CH10_PCM_TMATS_DATA_H_
