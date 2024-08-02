#ifndef CH10_PCM_TMATS_DATA_H_
#define CH10_PCM_TMATS_DATA_H_

class Ch10PCMTMATSData
{
    public:
        int common_word_length_;
        int word_transfer_order_;
        int parity_;
        int crc_;
        int crc_check_word_starting_bit_;
        int crc_data_start_bit_;
        int crc_data_number_of_bits_;
        int min_frames_in_maj_frame_;
        int words_in_min_frame_;
        int bits_in_min_frame_;

        Ch10PCMTMATSData() : common_word_length_(-1), word_transfer_order_(-1),
            parity_(-1), crc_(-1), crc_check_word_starting_bit_(-1), 
            crc_data_start_bit_(-1), crc_data_number_of_bits_(-1), 
            min_frames_in_maj_frame_(-1), words_in_min_frame_(-1),
            bits_in_min_frame_(-1)
        {}
};

#endif  // CH10_PCM_TMATS_DATA_H_
