#include <ctime>
#include <limits>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_packet_header_component.h"

class Ch10PacketHeaderComponentTest : public ::testing::Test
{
   protected:
    Ch10PacketHeaderFmt pkt_hdr_fmt_;
    Ch10Context ctx_;
    Ch10PacketHeaderComponent ch10_pkt_hdr_comp_;
    const uint8_t* data_ptr_;
    const uint8_t* orig_data_ptr_;
    const uint8_t* body_ptr_;
    Ch10Status status_;
    std::vector<uint8_t> fake_body_and_footer8_;
    std::vector<uint16_t> fake_body_and_footer16_;
    std::vector<uint32_t> fake_body_and_footer32_;
    uint64_t time_ns_;

    Ch10PacketHeaderComponentTest() : data_ptr_(nullptr), orig_data_ptr_(nullptr), status_(Ch10Status::NONE), body_ptr_(nullptr), ctx_(0, 0), ch10_pkt_hdr_comp_(&ctx_), time_ns_(0)
    {
        // Fill out values for fake header data.
        pkt_hdr_fmt_.sync = ch10_pkt_hdr_comp_.sync_;
        pkt_hdr_fmt_.chanID = 1;
        pkt_hdr_fmt_.pkt_size = 1000;
        pkt_hdr_fmt_.data_size = 800;
        pkt_hdr_fmt_.data_type_ver = 1;
        pkt_hdr_fmt_.seq_num = 0;
        pkt_hdr_fmt_.checksum_existence = 2;  // 16-bit header checksum
        pkt_hdr_fmt_.time_format = 0;
        pkt_hdr_fmt_.overflow_err = 0;
        pkt_hdr_fmt_.sync_err = 0;
        pkt_hdr_fmt_.intrapkt_ts_source = 0;
        pkt_hdr_fmt_.secondary_hdr = 0;
        pkt_hdr_fmt_.data_type = 0x01;  // TMATS
        pkt_hdr_fmt_.rtc1 = 0;          // set later
        pkt_hdr_fmt_.rtc2 = 0;          // set later
        pkt_hdr_fmt_.checksum = 0;      // set later
        srand(time(NULL));
    }

    ~Ch10PacketHeaderComponentTest()
    {
        //spdlog::shutdown();
    }

    uint16_t CalculateSecondaryHeaderChecksum(const uint8_t* hdr_data)
    {
        uint16_t checksum = 0;
        const uint16_t* sum_unit = (const uint16_t*)hdr_data;
        for (uint8_t i = 0; i < 4; i++)
            checksum += sum_unit[i];
        return checksum;
    }

    uint16_t Calculate16BitChecksum()
    {
        // Divide by two for 16bit checksum.
        uint8_t n_units = (sizeof(pkt_hdr_fmt_) - 2) / 2;
        const uint16_t* check_units = (const uint16_t*)&pkt_hdr_fmt_;
        uint16_t checksum = 0;
        for (uint8_t i = 0; i < n_units; i++)
            checksum += check_units[i];

        return checksum;
    }

    const uint8_t* CreateFakeBodyAndFooter8(int body_and_footer_byte_length,
                                            int secondary_header_present, int& corrected_total_pkt_len)
    {
        int corrected_body_and_footer_len = 0;
        CreateValidPacketLength(body_and_footer_byte_length,
                                secondary_header_present, corrected_body_and_footer_len,
                                corrected_total_pkt_len);

        //printf("corrected body and footer: %d, corrected total: %d\n",
        //corrected_body_and_footer_len, corrected_total_pkt_len);
        fake_body_and_footer8_.resize(corrected_body_and_footer_len);
        uint8_t sum = 0;
        // printf("values:\n");
        for (int i = 0; i < corrected_body_and_footer_len - 1; i++)
        {
            fake_body_and_footer8_[i] = (uint8_t)(rand() % std::numeric_limits<uint8_t>::max());
            // printf("%hhu, ", fake_body_and_footer8_[i]);
            sum += fake_body_and_footer8_[i];
        }
        fake_body_and_footer8_[corrected_body_and_footer_len - 1] = sum;
        // printf("\nsum: %hhu\n", sum);
        return fake_body_and_footer8_.data();
    }

    const uint8_t* CreateFakeBodyAndFooter16(int body_and_footer_byte_length,
                                             int secondary_header_present, int& corrected_total_pkt_len)
    {
        int corrected_body_and_footer_len = 0;
        CreateValidPacketLength(body_and_footer_byte_length,
                                secondary_header_present, corrected_body_and_footer_len,
                                corrected_total_pkt_len);

        int data_units = corrected_body_and_footer_len / 2;
        fake_body_and_footer16_.resize(data_units);
        uint16_t sum = 0;
        for (int i = 0; i < data_units - 1; i++)
        {
            fake_body_and_footer16_[i] = (uint16_t)(rand() % std::numeric_limits<uint16_t>::max());
            //printf("adding %hu\n", fake_body_and_footer16_[i]);
            sum += fake_body_and_footer16_[i];
        }
        fake_body_and_footer16_[data_units - 1] = sum;
        //printf("sum: %hhu\n", sum);
        return (const uint8_t*)fake_body_and_footer16_.data();
    }

    const uint8_t* CreateFakeBodyAndFooter32(int body_and_footer_byte_length,
                                             int secondary_header_present, int& corrected_total_pkt_len)
    {
        int corrected_body_and_footer_len = 0;
        CreateValidPacketLength(body_and_footer_byte_length,
                                secondary_header_present, corrected_body_and_footer_len,
                                corrected_total_pkt_len);

        int data_units = corrected_body_and_footer_len / 4;
        fake_body_and_footer32_.resize(data_units);
        uint32_t sum = 0;
        for (int i = 0; i < data_units - 1; i++)
        {
            fake_body_and_footer32_[i] = (uint32_t)rand();
            sum += fake_body_and_footer32_[i];
        }
        fake_body_and_footer32_[data_units - 1] = sum;
        return (const uint8_t*)fake_body_and_footer32_.data();
    }

    void CreateValidPacketLength(int body_and_footer_len, int sec_hdr_present,
                                 int& corrected_body_and_footer_len,
                                 int& corrected_total_pkt_len)
    {
        int total_pkt_size = 0;
        if (sec_hdr_present)
        {
            // secondary header length = 12 bytes
            // primary header length = 24 bytes
            total_pkt_size = body_and_footer_len + 12 + 24;
        }
        else
        {
            total_pkt_size = body_and_footer_len + 24;
        }

        corrected_total_pkt_len = (total_pkt_size / 4) * 4;
        corrected_body_and_footer_len = corrected_total_pkt_len;
        if (sec_hdr_present)
            corrected_body_and_footer_len -= (12 + 24);
        else
            corrected_body_and_footer_len -= 24;
        //printf("orig body and footer len: %d, corrected packet len: %d, corrected body and footer len: %d\n",
        //   body_and_footer_len, corrected_total_pkt_len, corrected_body_and_footer_len);
    }
};

TEST_F(Ch10PacketHeaderComponentTest, ParseGoodSync)
{
    // Parse pre-defined fake header and check data.
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;
    orig_data_ptr_ = data_ptr_;

    // Calculate and insert correct checksum such that
    // no checksum errors are returned.
    uint16_t checksum = Calculate16BitChecksum();
    pkt_hdr_fmt_.checksum = checksum;

    status_ = ch10_pkt_hdr_comp_.Parse(data_ptr_);

    // Check a few elements.
    EXPECT_EQ((*ch10_pkt_hdr_comp_.std_hdr_elem.element)->chanID, 1);
    EXPECT_EQ((*ch10_pkt_hdr_comp_.std_hdr_elem.element)->data_size, 800);
    EXPECT_EQ((*ch10_pkt_hdr_comp_.std_hdr_elem.element)->data_type, 0x01);

    // Check that pointer has been incremented.
    EXPECT_EQ(data_ptr_, orig_data_ptr_ + ch10_pkt_hdr_comp_.std_hdr_elem.size);

    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketHeaderComponentTest, ParseBadSync)
{
    // Set bad sync value.
    pkt_hdr_fmt_.sync = 10;

    // Parse pre-defined fake header and check data.
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;
    status_ = ch10_pkt_hdr_comp_.Parse(data_ptr_);

    // There is no need to check that the raw bits are parsed
    // correctly, as is done in ParseGoodSync since the basic
    // functionality of ParseElements is tested in ch10_packet_component_u.cpp.

    EXPECT_EQ(status_, Ch10Status::BAD_SYNC);
}

TEST_F(Ch10PacketHeaderComponentTest, ParseSecondaryHeaderNotPresent)
{
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 0;

    // Calculate and insert correct checksum such that
    // no checksum errors are returned.
    uint16_t checksum = Calculate16BitChecksum();
    pkt_hdr_fmt_.checksum = checksum;

    // Parse pre-defined fake header and check data.
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;
    orig_data_ptr_ = data_ptr_;
    status_ = ch10_pkt_hdr_comp_.Parse(data_ptr_);
    EXPECT_EQ(status_, Ch10Status::OK);

    status_ = ch10_pkt_hdr_comp_.ParseSecondaryHeader(data_ptr_, time_ns_);
    EXPECT_EQ(status_, Ch10Status::OK);
    EXPECT_EQ(0, time_ns_);
}

TEST_F(Ch10PacketHeaderComponentTest, ParseSecondaryHeaderInvalidFormat)
{
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 1;
    pkt_hdr_fmt_.time_format = 3;  // invalid

    // Calculate and insert correct checksum such that
    // no checksum errors are returned.
    uint16_t checksum = Calculate16BitChecksum();
    pkt_hdr_fmt_.checksum = checksum;

    // Parse pre-defined fake header and check data.
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;
    orig_data_ptr_ = data_ptr_;
    status_ = ch10_pkt_hdr_comp_.Parse(data_ptr_);
    EXPECT_EQ(status_, Ch10Status::OK);

    status_ = ch10_pkt_hdr_comp_.ParseSecondaryHeader(data_ptr_, time_ns_);
    EXPECT_EQ(status_, Ch10Status::INVALID_SECONDARY_HDR_FMT);
    EXPECT_EQ(0, time_ns_);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyHeaderChecksumTrue)
{
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;

    // Calculate checksum value and insert it into the checksum field.
    uint16_t checksum = Calculate16BitChecksum();
    pkt_hdr_fmt_.checksum = checksum;

    status_ = ch10_pkt_hdr_comp_.VerifyHeaderChecksum(data_ptr_, pkt_hdr_fmt_.checksum);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_TRUE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyHeaderChecksumFalse)
{
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;

    // Calculate checksum value and insert it into the checksum field.
    uint16_t checksum = 13221;
    pkt_hdr_fmt_.checksum = checksum;

    status_ = ch10_pkt_hdr_comp_.VerifyHeaderChecksum(data_ptr_, pkt_hdr_fmt_.checksum);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_FALSE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksumNoChecksum)
{
    pkt_hdr_fmt_.checksum_existence = 0;  // no checksum present

    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_, pkt_hdr_fmt_.checksum_existence,
                                                    1000, 1);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_NOT_PRESENT);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksum8BitNoSecondaryTrue)
{
    pkt_hdr_fmt_.checksum_existence = 1;  // 8-bit
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 0;

    int body_and_footer_len = 30;
    int corr_total_pkt_len = 0;
    body_ptr_ = CreateFakeBodyAndFooter8(body_and_footer_len, pkt_hdr_fmt_.secondary_hdr,
                                         corr_total_pkt_len);

    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_,
                                                    pkt_hdr_fmt_.checksum_existence, corr_total_pkt_len, pkt_hdr_fmt_.secondary_hdr);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_TRUE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksum8BitNoSecondaryFalse)
{
    pkt_hdr_fmt_.checksum_existence = 1;  // 8-bit
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 0;

    int body_and_footer_len = 36;
    int corr_total_pkt_len = 0;
    body_ptr_ = CreateFakeBodyAndFooter8(body_and_footer_len, pkt_hdr_fmt_.secondary_hdr,
                                         corr_total_pkt_len);

    // Add bad values by using incorrect packet length.
    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_,
                                                    pkt_hdr_fmt_.checksum_existence, corr_total_pkt_len - 3, pkt_hdr_fmt_.secondary_hdr);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_FALSE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksum8BitWithSecondaryTrue)
{
    pkt_hdr_fmt_.checksum_existence = 1;  // 8-bit
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 1;

    int body_and_footer_len = 55;
    int corr_total_pkt_len = 0;
    body_ptr_ = CreateFakeBodyAndFooter8(body_and_footer_len, pkt_hdr_fmt_.secondary_hdr,
                                         corr_total_pkt_len);

    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_,
                                                    pkt_hdr_fmt_.checksum_existence, corr_total_pkt_len, pkt_hdr_fmt_.secondary_hdr);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_TRUE);
}

// Note that I'm not testing WithSecondaryFalse. It seems excessive to check
// all the cases in which the sums don't match. More tests can be added if this
// turns out to be a poor assumption.

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksum16BitNoSecondaryTrue)
{
    pkt_hdr_fmt_.checksum_existence = 2;  // 16-bit
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 0;

    int body_and_footer_len = 300;
    int corr_total_pkt_len = 0;
    body_ptr_ = CreateFakeBodyAndFooter16(body_and_footer_len, pkt_hdr_fmt_.secondary_hdr,
                                          corr_total_pkt_len);

    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_,
                                                    pkt_hdr_fmt_.checksum_existence, corr_total_pkt_len, pkt_hdr_fmt_.secondary_hdr);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_TRUE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksum16BitNoSecondaryFalse)
{
    pkt_hdr_fmt_.checksum_existence = 2;  // 16-bit
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 0;

    int body_and_footer_len = 222;
    int corr_total_pkt_len = 0;
    body_ptr_ = CreateFakeBodyAndFooter16(body_and_footer_len, pkt_hdr_fmt_.secondary_hdr,
                                          corr_total_pkt_len);

    // Add bad values by using incorrect packet length.
    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_,
                                                    pkt_hdr_fmt_.checksum_existence, corr_total_pkt_len - 2, pkt_hdr_fmt_.secondary_hdr);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_FALSE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksum16BitWithSecondaryTrue)
{
    pkt_hdr_fmt_.checksum_existence = 2;  // 16-bit
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 1;

    int body_and_footer_len = 478;
    int corr_total_pkt_len = 0;
    body_ptr_ = CreateFakeBodyAndFooter16(body_and_footer_len, pkt_hdr_fmt_.secondary_hdr,
                                          corr_total_pkt_len);

    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_,
                                                    pkt_hdr_fmt_.checksum_existence, corr_total_pkt_len, pkt_hdr_fmt_.secondary_hdr);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_TRUE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksum32BitNoSecondaryTrue)
{
    pkt_hdr_fmt_.checksum_existence = 3;  // 32-bit
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 0;

    int body_and_footer_len = 3000;
    int corr_total_pkt_len = 0;
    body_ptr_ = CreateFakeBodyAndFooter32(body_and_footer_len, pkt_hdr_fmt_.secondary_hdr,
                                          corr_total_pkt_len);

    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_,
                                                    pkt_hdr_fmt_.checksum_existence, corr_total_pkt_len, pkt_hdr_fmt_.secondary_hdr);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_TRUE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksum32BitNoSecondaryFalse)
{
    pkt_hdr_fmt_.checksum_existence = 3;  // 32-bit
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 0;

    int body_and_footer_len = 7252;
    int corr_total_pkt_len = 0;
    body_ptr_ = CreateFakeBodyAndFooter32(body_and_footer_len, pkt_hdr_fmt_.secondary_hdr,
                                          corr_total_pkt_len);

    // Add bad values by using incorrect packet length.
    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_,
                                                    pkt_hdr_fmt_.checksum_existence, corr_total_pkt_len + 4, pkt_hdr_fmt_.secondary_hdr);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_FALSE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifyDataChecksum32BitWithSecondaryTrue)
{
    pkt_hdr_fmt_.checksum_existence = 3;  // 32-bit
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 1;

    int body_and_footer_len = 5513;
    int corr_total_pkt_len = 0;
    body_ptr_ = CreateFakeBodyAndFooter32(body_and_footer_len, pkt_hdr_fmt_.secondary_hdr,
                                          corr_total_pkt_len);

    status_ = ch10_pkt_hdr_comp_.VerifyDataChecksum(body_ptr_,
                                                    pkt_hdr_fmt_.checksum_existence, corr_total_pkt_len, pkt_hdr_fmt_.secondary_hdr);
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_TRUE);
}

TEST_F(Ch10PacketHeaderComponentTest, VerifySecondaryHeaderChecksumTrue)
{
    std::vector<uint8_t> header_data = {23, 45, 67, 89, 76, 54, 32, 11};
    const uint8_t* header_data_ptr = (const uint8_t*)header_data.data();
    status_ = ch10_pkt_hdr_comp_.VerifySecondaryHeaderChecksum(header_data_ptr,
                                                               CalculateSecondaryHeaderChecksum(header_data_ptr));
    EXPECT_EQ(status_, Ch10Status::CHECKSUM_TRUE);
}