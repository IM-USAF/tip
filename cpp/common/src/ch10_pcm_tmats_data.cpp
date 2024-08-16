#include "ch10_pcm_tmats_data.h"

const std::string Ch10PCMTMATSData::null_indicator_ = "null";

Ch10PCMTMATSData& Ch10PCMTMATSData::operator=(const Ch10PCMTMATSData& input)
{
    this->data_link_name_ = input.data_link_name_;
    this->pcm_code_ = input.pcm_code_;
    this->encrypted_ = input.encrypted_;
    this->bit_rate_ = input.bit_rate_;
    this->polarity_ = input.polarity_;
    this->auto_pol_correction_ = input.auto_pol_correction_;
    this->data_direction_ = input.data_direction_;
    this->data_randomized_ = input.data_randomized_;
    this->randomizer_type_ = input.randomizer_type_;
    this->type_format_ = input.type_format_;
    this->common_word_length_ = input.common_word_length_;
    this->word_transfer_order_ = input.word_transfer_order_;
    this->parity_ = input.parity_;
    this->parity_transfer_order_ = input.parity_transfer_order_;
    this->crc_ = input.crc_;
    this->crc_check_word_starting_bit_ = input.crc_check_word_starting_bit_;
    this->crc_data_number_of_bits_ = input.crc_data_number_of_bits_;
    this->crc_data_start_bit_ = input.crc_data_start_bit_;
    this->min_frames_in_maj_frame_ = input.min_frames_in_maj_frame_;
    this->words_in_min_frame_ = input.words_in_min_frame_;
    this->bits_in_min_frame_ = input.bits_in_min_frame_;
    this->sync_type_ = input.sync_type_;
}
