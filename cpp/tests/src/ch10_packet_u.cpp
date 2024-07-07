#include "gtest/gtest.h"
#include "gmock/gmock.h"

// Important to include the following two
// headers prior to binbuff.h, which includes
// spdlog. There is a redefinition error with
// some of the types defined in Arrow.
#include "ch10_context.h"
#include "ch10_packet.h"
#include "binbuff.h"
#include "binbuff_mock.h"
#include "ch10_packet_header_component_mock.h"
#include "ch10_context_mock.h"
#include "ch10_time_mock.h"
#include "ch10_tmats_component_mock.h"
#include "ch10_tdp_component_mock.h"
#include "ch10_1553f1_component_mock.h"
#include "ch10_videof0_component_mock.h"
#include "ch10_ethernetf0_component_mock.h"
#include "ch10_arinc429f0_component_mock.h"

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::_;

class Ch10PacketTest : public ::testing::Test
{
   protected:
    NiceMock<MockBinBuff> mock_bb_;
    NiceMock<MockCh10PacketHeaderComponent> mock_hdr_;
    NiceMock<MockCh10Context> mock_ctx_;
    NiceMock<MockCh10Time> mock_ch10_time_;
    Ch10Status status_;
    NiceMock<MockCh10TMATSComponent> mock_tmats_;
    NiceMock<MockCh10TDPComponent> mock_tdp_;
    NiceMock<MockCh101553F1Component> mock_milstd1553_;
    NiceMock<MockCh10VideoF0Component> mock_vid_;
    NiceMock<MockCh10EthernetF0Component> mock_eth_;
    NiceMock<MockCh10429F0Component> mock_arinc429_;
    Ch10Packet p_;
    uint64_t abs_pos_;
    bool found_tmats_;

    Ch10PacketTest() : status_(Ch10Status::NONE), mock_bb_(), mock_ctx_(), mock_ch10_time_(),
        p_(&mock_bb_, &mock_ctx_, &mock_ch10_time_),
        mock_tmats_(&mock_ctx_), mock_tdp_(&mock_ctx_), mock_milstd1553_(&mock_ctx_), mock_vid_(&mock_ctx_),
        mock_eth_(&mock_ctx_), mock_arinc429_(&mock_ctx_), mock_hdr_(&mock_ctx_)
    {}

    virtual void SetUp()
    {
        p_.SetCh10ComponentParsers(&mock_hdr_, &mock_tmats_, &mock_tdp_, &mock_milstd1553_,
            &mock_vid_, &mock_eth_, &mock_arinc429_);
    }
};

TEST_F(Ch10PacketTest, AdvanceBufferBinBuffGoodAdvance)
{
    uint64_t advance_by = 41132100;
    EXPECT_CALL(mock_bb_, AdvanceReadPos(advance_by))
        .WillOnce(Return(0));

    status_ = p_.AdvanceBuffer(advance_by);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, AdvanceBufferABinBuffBadAdvance)
{
    uint64_t advance_by = 32100;
    EXPECT_CALL(mock_bb_, AdvanceReadPos(advance_by))
        .WillOnce(Return(1));

    status_ = p_.AdvanceBuffer(advance_by);
    EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusOK)
{
    uint64_t pkt_size = 1839;

    status_ = Ch10Status::OK;
    status_ = p_.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusBadSyncNotBufferLimited)
{
    uint64_t pkt_size = 1839;

    // BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
    // This function will also make a call to Ch10Context.

    // The buffer will return 0 to indicate that it can advance by the
    // requested amount.
    EXPECT_CALL(mock_bb_, AdvanceReadPos(1))
        .WillOnce(Return(0));

    status_ = Ch10Status::BAD_SYNC;
    status_ = p_.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::BAD_SYNC);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusBadSyncBufferLimited)
{
    uint64_t pkt_size = 1839;

    // BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
    // This function will also make a call to Ch10Context.

    // The buffer will return 1 to indicate that it can't advance by
    // requested amount.
    EXPECT_CALL(mock_bb_, AdvanceReadPos(1))
        .WillOnce(Return(1));

    status_ = Ch10Status::BAD_SYNC;
    status_ = p_.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusNotOKDefaultNotBufferLimited)
{
    uint64_t pkt_size = 1839;

    // BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
    // This function will also make a call to Ch10Context.

    // The buffer will return 0 to indicate that it can advance by the
    // requested amount.
    EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
        .WillOnce(Return(0));

    status_ = Ch10Status::PKT_TYPE_EXIT;
    status_ = p_.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::PKT_TYPE_NO);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusNotOKDefaultBufferLimited)
{
    uint64_t pkt_size = 1839;

    // BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
    // This function will also make a call to Ch10Context.

    // The buffer will return 1 to indicate that it can't advance by
    // requested amount.
    EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
        .WillOnce(Return(1));

    status_ = Ch10Status::PKT_TYPE_EXIT;
    status_ = p_.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ManageSecondaryHeaderParseStatusOK)
{
    uint64_t pkt_size = 1839;

    status_ = Ch10Status::OK;
    status_ = p_.ManageSecondaryHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, ManageSecondaryHeaderParseStatusInvalidNotBufferLimited)
{
    uint64_t pkt_size = 1839;

    EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
        .WillOnce(Return(0));

    status_ = Ch10Status::INVALID_SECONDARY_HDR_FMT;
    status_ = p_.ManageSecondaryHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::PKT_TYPE_NO);
}

TEST_F(Ch10PacketTest, ManageSecondaryHeaderParseStatusInvalidBufferLimited)
{
    uint64_t pkt_size = 1839;

    EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
        .WillOnce(Return(1));

    status_ = Ch10Status::INVALID_SECONDARY_HDR_FMT;
    status_ = p_.ManageSecondaryHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ParseHeaderBytesAvailableFail)
{
    EXPECT_CALL(mock_bb_, BytesAvailable(mock_hdr_.std_hdr_size_)).WillOnce(Return(false));
    ASSERT_EQ(Ch10Status::BUFFER_LIMITED, p_.ParseHeader());
}

TEST_F(Ch10PacketTest, ParseHeaderManageHeaderParseStatusFail)
{
    ::testing::Sequence seq;
    EXPECT_CALL(mock_bb_, BytesAvailable(mock_hdr_.std_hdr_size_)).InSequence(seq)
        .WillOnce(Return(true));

    uint8_t some_data = 3;
    const uint8_t* data_ptr = static_cast<const uint8_t*>(&some_data);
    EXPECT_CALL(mock_bb_, Data()).InSequence(seq).WillOnce(Return(data_ptr));
    EXPECT_CALL(mock_hdr_, Parse(data_ptr)).InSequence(seq).WillOnce(Return(Ch10Status::BAD_SYNC));

    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.pkt_size = 10;
    EXPECT_CALL(mock_hdr_, GetHeader()).InSequence(seq).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_bb_, AdvanceReadPos(1)).InSequence(seq).WillOnce(Return(1));

    ASSERT_EQ(Ch10Status::BUFFER_LIMITED, p_.ParseHeader());
}

TEST_F(Ch10PacketTest, ParseHeaderAllPacketBytesAvailableFail)
{
    ::testing::Sequence seq;
    EXPECT_CALL(mock_bb_, BytesAvailable(mock_hdr_.std_hdr_size_)).InSequence(seq)
        .WillOnce(Return(true));

    uint8_t some_data = 3;
    const uint8_t* data_ptr = static_cast<const uint8_t*>(&some_data);
    EXPECT_CALL(mock_bb_, Data()).InSequence(seq).WillOnce(Return(data_ptr));
    EXPECT_CALL(mock_hdr_, Parse(data_ptr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));

    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.pkt_size = 10;
    EXPECT_CALL(mock_hdr_, GetHeader()).InSequence(seq).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_bb_, BytesAvailable(hdr_fmt.pkt_size)).InSequence(seq)
        .WillOnce(Return(false));

    ASSERT_EQ(Ch10Status::BUFFER_LIMITED, p_.ParseHeader());
}

TEST_F(Ch10PacketTest, ParseHeaderAdvanceBufferFail)
{
    ::testing::Sequence seq;
    EXPECT_CALL(mock_bb_, BytesAvailable(mock_hdr_.std_hdr_size_)).InSequence(seq)
        .WillOnce(Return(true));

    uint8_t some_data = 3;
    const uint8_t* data_ptr = static_cast<const uint8_t*>(&some_data);
    EXPECT_CALL(mock_bb_, Data()).InSequence(seq).WillOnce(Return(data_ptr));
    EXPECT_CALL(mock_hdr_, Parse(data_ptr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));

    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.pkt_size = 10;
    EXPECT_CALL(mock_hdr_, GetHeader()).InSequence(seq).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_bb_, BytesAvailable(hdr_fmt.pkt_size)).InSequence(seq)
        .WillOnce(Return(true));
    EXPECT_CALL(mock_bb_, AdvanceReadPos(hdr_fmt.pkt_size)).InSequence(seq).WillOnce(Return(1));

    ASSERT_EQ(Ch10Status::BUFFER_LIMITED, p_.ParseHeader());
}

TEST_F(Ch10PacketTest, ParseHeaderUpdateContextFail)
{
    ::testing::Sequence seq;
    EXPECT_CALL(mock_bb_, BytesAvailable(mock_hdr_.std_hdr_size_)).InSequence(seq)
        .WillOnce(Return(true));

    uint8_t some_data = 3;
    const uint8_t* data_ptr = static_cast<const uint8_t*>(&some_data);
    EXPECT_CALL(mock_bb_, Data()).InSequence(seq).WillOnce(Return(data_ptr));
    EXPECT_CALL(mock_hdr_, Parse(data_ptr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));

    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.pkt_size = 10;
    hdr_fmt.rtc1 = 29439;
    hdr_fmt.rtc2 = 58828;
    EXPECT_CALL(mock_hdr_, GetHeader()).InSequence(seq).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_bb_, BytesAvailable(hdr_fmt.pkt_size)).InSequence(seq)
        .WillOnce(Return(true));
    EXPECT_CALL(mock_bb_, AdvanceReadPos(hdr_fmt.pkt_size)).InSequence(seq).WillOnce(Return(0));
    uint64_t temp_time = 193284838;
    EXPECT_CALL(mock_ch10_time_, CalculateRTCTimeFromComponents(hdr_fmt.rtc1, hdr_fmt.rtc2))
        .InSequence(seq).WillOnce(::testing::ReturnRef(temp_time));
    EXPECT_CALL(mock_ctx_, UpdateContext(_, &hdr_fmt, _)).
        InSequence(seq).WillOnce(Return(Ch10Status::NONE)); // Not OK

    ASSERT_EQ(Ch10Status::PKT_TYPE_NO, p_.ParseHeader());
}

TEST_F(Ch10PacketTest, ParseHeaderVerifyDataChecksumFail)
{
    ::testing::Sequence seq;
    EXPECT_CALL(mock_bb_, BytesAvailable(mock_hdr_.std_hdr_size_)).InSequence(seq)
        .WillOnce(Return(true));

    uint8_t some_data = 3;
    const uint8_t* data_ptr = static_cast<const uint8_t*>(&some_data);
    EXPECT_CALL(mock_bb_, Data()).InSequence(seq).WillOnce(Return(data_ptr));
    EXPECT_CALL(mock_hdr_, Parse(data_ptr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));

    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.pkt_size = 10;
    hdr_fmt.rtc1 = 29439;
    hdr_fmt.rtc2 = 58428;
    EXPECT_CALL(mock_hdr_, GetHeader()).InSequence(seq).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_bb_, BytesAvailable(hdr_fmt.pkt_size)).InSequence(seq)
        .WillOnce(Return(true));
    EXPECT_CALL(mock_bb_, AdvanceReadPos(hdr_fmt.pkt_size)).InSequence(seq).WillOnce(Return(0));
    uint64_t temp_time = 193284838;
    EXPECT_CALL(mock_ch10_time_, CalculateRTCTimeFromComponents(hdr_fmt.rtc1, hdr_fmt.rtc2))
        .InSequence(seq).WillOnce(::testing::ReturnRef(temp_time));
    EXPECT_CALL(mock_ctx_, UpdateContext(_, &hdr_fmt, _)).
        InSequence(seq).WillOnce(Return(Ch10Status::OK));
    EXPECT_CALL(mock_hdr_, VerifyDataChecksum(_, hdr_fmt.checksum_existence, hdr_fmt.pkt_size,
        hdr_fmt.secondary_hdr)).InSequence(seq).WillOnce(Return(Ch10Status::CHECKSUM_FALSE));

    ASSERT_EQ(Ch10Status::PKT_TYPE_NO, p_.ParseHeader());
}

TEST_F(Ch10PacketTest, ParseHeaderSecondaryHeaderFail)
{
    ::testing::Sequence seq;
    EXPECT_CALL(mock_bb_, BytesAvailable(mock_hdr_.std_hdr_size_)).InSequence(seq)
        .WillOnce(Return(true));

    uint8_t some_data = 3;
    const uint8_t* data_ptr = static_cast<const uint8_t*>(&some_data);
    EXPECT_CALL(mock_bb_, Data()).InSequence(seq).WillOnce(Return(data_ptr));
    EXPECT_CALL(mock_hdr_, Parse(data_ptr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));

    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.pkt_size = 10;
    hdr_fmt.rtc1 = 29439;
    hdr_fmt.rtc2 = 58828;
    EXPECT_CALL(mock_hdr_, GetHeader()).InSequence(seq).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_bb_, BytesAvailable(hdr_fmt.pkt_size)).InSequence(seq)
        .WillOnce(Return(true));
    EXPECT_CALL(mock_bb_, AdvanceReadPos(hdr_fmt.pkt_size)).InSequence(seq).WillOnce(Return(0));
    uint64_t temp_time = 193284838;
    EXPECT_CALL(mock_ch10_time_, CalculateRTCTimeFromComponents(hdr_fmt.rtc1, hdr_fmt.rtc2))
        .InSequence(seq).WillOnce(::testing::ReturnRef(temp_time));
    EXPECT_CALL(mock_ctx_, UpdateContext(_, &hdr_fmt, _)).
        InSequence(seq).WillOnce(Return(Ch10Status::OK));
    EXPECT_CALL(mock_hdr_, VerifyDataChecksum(_, hdr_fmt.checksum_existence, hdr_fmt.pkt_size,
        hdr_fmt.secondary_hdr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));
    EXPECT_CALL(mock_hdr_, ParseSecondaryHeader(_, _)).InSequence(seq)
        .WillOnce(Return(Ch10Status::CHECKSUM_FALSE));
    EXPECT_CALL(mock_bb_, AdvanceReadPos(hdr_fmt.pkt_size)).InSequence(seq).WillOnce(Return(0));
    EXPECT_CALL(mock_ctx_, AdvanceAbsPos(hdr_fmt.pkt_size)).InSequence(seq).WillOnce(Return());

    ASSERT_EQ(Ch10Status::PKT_TYPE_NO, p_.ParseHeader());
}

TEST_F(Ch10PacketTest, ParseHeaderNoUpdateSecondaryHeaderTime)
{
    ::testing::Sequence seq;
    EXPECT_CALL(mock_bb_, BytesAvailable(mock_hdr_.std_hdr_size_)).InSequence(seq)
        .WillOnce(Return(true));

    uint8_t some_data = 3;
    const uint8_t* data_ptr = static_cast<const uint8_t*>(&some_data);
    EXPECT_CALL(mock_bb_, Data()).InSequence(seq).WillOnce(Return(data_ptr));
    EXPECT_CALL(mock_hdr_, Parse(data_ptr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));

    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.pkt_size = 10;
    hdr_fmt.rtc1 = 29439;
    hdr_fmt.rtc2 = 58428;
    EXPECT_CALL(mock_hdr_, GetHeader()).InSequence(seq).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_bb_, BytesAvailable(hdr_fmt.pkt_size)).InSequence(seq)
        .WillOnce(Return(true));
    EXPECT_CALL(mock_bb_, AdvanceReadPos(hdr_fmt.pkt_size)).InSequence(seq).WillOnce(Return(0));
    uint64_t temp_time = 193284838;
    EXPECT_CALL(mock_ch10_time_, CalculateRTCTimeFromComponents(hdr_fmt.rtc1, hdr_fmt.rtc2))
        .InSequence(seq).WillOnce(::testing::ReturnRef(temp_time));
    EXPECT_CALL(mock_ctx_, UpdateContext(_, &hdr_fmt, _)).
        InSequence(seq).WillOnce(Return(Ch10Status::OK));
    EXPECT_CALL(mock_hdr_, VerifyDataChecksum(_, hdr_fmt.checksum_existence, hdr_fmt.pkt_size,
        hdr_fmt.secondary_hdr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));
    uint64_t sec_hdr_time_ns = 0;
    EXPECT_CALL(mock_hdr_, ParseSecondaryHeader(_, _)).InSequence(seq)
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(sec_hdr_time_ns),
        Return(Ch10Status::OK)));
    EXPECT_CALL(mock_ctx_, ContinueWithPacketType(hdr_fmt.data_type)).InSequence(seq)
        .WillOnce(Return(Ch10Status::PKT_TYPE_YES));

    ASSERT_EQ(Ch10Status::PKT_TYPE_YES, p_.ParseHeader());
}

TEST_F(Ch10PacketTest, ParseHeaderUpdateSecondaryHeaderTime)
{
    ::testing::Sequence seq;
    EXPECT_CALL(mock_bb_, BytesAvailable(mock_hdr_.std_hdr_size_)).InSequence(seq)
        .WillOnce(Return(true));

    uint8_t some_data = 3;
    const uint8_t* data_ptr = static_cast<const uint8_t*>(&some_data);
    EXPECT_CALL(mock_bb_, Data()).InSequence(seq).WillOnce(Return(data_ptr));
    EXPECT_CALL(mock_hdr_, Parse(data_ptr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));

    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.pkt_size = 10;
    hdr_fmt.rtc1 = 29439;
    hdr_fmt.rtc2 = 58488;
    EXPECT_CALL(mock_hdr_, GetHeader()).InSequence(seq).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_bb_, BytesAvailable(hdr_fmt.pkt_size)).InSequence(seq)
        .WillOnce(Return(true));
    EXPECT_CALL(mock_bb_, AdvanceReadPos(hdr_fmt.pkt_size)).InSequence(seq).WillOnce(Return(0));
    uint64_t temp_time = 193284838;
    EXPECT_CALL(mock_ch10_time_, CalculateRTCTimeFromComponents(hdr_fmt.rtc1, hdr_fmt.rtc2))
        .InSequence(seq).WillOnce(::testing::ReturnRef(temp_time));
    EXPECT_CALL(mock_ctx_, UpdateContext(_, &hdr_fmt, _)).
        InSequence(seq).WillOnce(Return(Ch10Status::OK));
    EXPECT_CALL(mock_hdr_, VerifyDataChecksum(_, hdr_fmt.checksum_existence, hdr_fmt.pkt_size,
        hdr_fmt.secondary_hdr)).InSequence(seq).WillOnce(Return(Ch10Status::OK));
    uint64_t sec_hdr_time_ns = 8482010; // non-zero
    EXPECT_CALL(mock_hdr_, ParseSecondaryHeader(_, _)).InSequence(seq)
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(sec_hdr_time_ns),
        Return(Ch10Status::OK)));
    EXPECT_CALL(mock_ctx_, UpdateWithSecondaryHeaderTime(sec_hdr_time_ns)).InSequence(seq)
        .WillOnce(Return());
    EXPECT_CALL(mock_ctx_, ContinueWithPacketType(hdr_fmt.data_type)).InSequence(seq)
        .WillOnce(Return(Ch10Status::PKT_TYPE_YES));

    ASSERT_EQ(Ch10Status::PKT_TYPE_YES, p_.ParseHeader());
}

TEST_F(Ch10PacketTest, ParseBodyComputerGeneratedDataF1)
{
    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.data_type = static_cast<uint32_t>(Ch10PacketType::COMPUTER_GENERATED_DATA_F1);
    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::COMPUTER_GENERATED_DATA_F1))
        .WillOnce(Return(false));

    abs_pos_ = 0;
    found_tmats_ = false;
    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::NONE, p_.current_pkt_type);

    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::COMPUTER_GENERATED_DATA_F1))
        .WillOnce(Return(true));
    EXPECT_CALL(mock_tmats_, Parse(_)).WillOnce(Return(Ch10Status::OK));

    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::COMPUTER_GENERATED_DATA_F1, p_.current_pkt_type);
}

TEST_F(Ch10PacketTest, ParseBodyTimeDataF1)
{
    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.data_type = static_cast<uint32_t>(Ch10PacketType::TIME_DATA_F1);
    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::TIME_DATA_F1))
        .WillOnce(Return(false));

    abs_pos_ = 38382880;
    found_tmats_ = false;
    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::NONE, p_.current_pkt_type);

    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::TIME_DATA_F1))
        .WillOnce(Return(true));
    EXPECT_CALL(mock_tdp_, Parse(_)).WillOnce(Return(Ch10Status::OK));

    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::TIME_DATA_F1, p_.current_pkt_type);
}

TEST_F(Ch10PacketTest, ParseBodyMilStd1553F1)
{
    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.data_type = static_cast<uint32_t>(Ch10PacketType::MILSTD1553_F1);
    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::MILSTD1553_F1))
        .WillOnce(Return(false));

    abs_pos_ = 38382880;
    found_tmats_ = false;
    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::NONE, p_.current_pkt_type);

    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::MILSTD1553_F1))
        .WillOnce(Return(true));
    EXPECT_CALL(mock_milstd1553_, Parse(_)).WillOnce(Return(Ch10Status::OK));

    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::MILSTD1553_F1, p_.current_pkt_type);
}

TEST_F(Ch10PacketTest, ParseBodyVideoF0)
{
    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.data_type = static_cast<uint32_t>(Ch10PacketType::VIDEO_DATA_F0);
    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::VIDEO_DATA_F0))
        .WillOnce(Return(false));

    abs_pos_ = 38382880;
    found_tmats_ = false;
    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::NONE, p_.current_pkt_type);

    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::VIDEO_DATA_F0))
        .WillOnce(Return(true));
    EXPECT_CALL(mock_vid_, Parse(_)).WillOnce(Return(Ch10Status::OK));
    EXPECT_CALL(mock_ctx_, RecordMinVideoTimeStamp(_)).WillOnce(Return());

    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::VIDEO_DATA_F0, p_.current_pkt_type);
}

TEST_F(Ch10PacketTest, ParseBodyEthernetF0)
{
    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.data_type = static_cast<uint32_t>(Ch10PacketType::ETHERNET_DATA_F0);
    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::ETHERNET_DATA_F0))
        .WillOnce(Return(false));

    abs_pos_ = 38382880;
    found_tmats_ = false;
    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::NONE, p_.current_pkt_type);

    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::ETHERNET_DATA_F0))
        .WillOnce(Return(true));
    EXPECT_CALL(mock_eth_, Parse(_)).WillOnce(Return(Ch10Status::OK));

    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::ETHERNET_DATA_F0, p_.current_pkt_type);
}

TEST_F(Ch10PacketTest, ParseBody429F0)
{
    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.data_type = static_cast<uint32_t>(Ch10PacketType::ARINC429_F0);
    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::ARINC429_F0))
        .WillOnce(Return(false));

    abs_pos_ = 38382880;
    found_tmats_ = false;
    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::NONE, p_.current_pkt_type);

    EXPECT_CALL(mock_hdr_, GetHeader()).WillOnce(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, IsPacketTypeEnabled(Ch10PacketType::ARINC429_F0))
        .WillOnce(Return(true));
    EXPECT_CALL(mock_arinc429_, Parse(_)).WillOnce(Return(Ch10Status::OK));

    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::ARINC429_F0, p_.current_pkt_type);
}

TEST_F(Ch10PacketTest, ParseBodyDefault)
{
    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.data_type = static_cast<uint32_t>(Ch10PacketType::NONE);
    EXPECT_CALL(mock_hdr_, GetHeader()).WillRepeatedly(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, RegisterUnhandledPacketType(Ch10PacketType::NONE))
        .WillOnce(Return(false));

    abs_pos_ = 38382880;
    found_tmats_ = false;
    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::NONE, p_.current_pkt_type);

    EXPECT_CALL(mock_hdr_, GetHeader()).WillRepeatedly(Return(&hdr_fmt));
    EXPECT_CALL(mock_ctx_, RegisterUnhandledPacketType(Ch10PacketType::NONE))
        .WillOnce(Return(true));

    p_.ParseBody(abs_pos_, found_tmats_);
    EXPECT_EQ(Ch10PacketType::NONE, p_.current_pkt_type);
}

TEST_F(Ch10PacketTest, TmatsStatusSecondaryWorkerCurrTMATs)
{
    // non-zero indicates a ssecondary worker, i.e., not the first
    // worker which is expected to find TMATS as the first packet.
    abs_pos_ = 388282; 

    // a new worker will always initialize found_tmats to false.
    found_tmats_ = false;
    ASSERT_EQ(p_.TmatsStatus(abs_pos_, found_tmats_, true), 
        Ch10Status::TMATS_PKT_ERR);

    // However, just in case, TMATs shouldn't be found by a secondary
    // worker in any conditions.
    found_tmats_ = true;
    ASSERT_EQ(p_.TmatsStatus(abs_pos_, found_tmats_, true), 
        Ch10Status::TMATS_PKT_ERR);
}

TEST_F(Ch10PacketTest, TmatsStatusSecondaryWorkerNotCurrTMATs)
{
    // non-zero indicates a ssecondary worker, i.e., not the first
    // worker which is expected to find TMATS as the first packet.
    abs_pos_ = 388282; 

    // a new worker will always initialize found_tmats to false.
    found_tmats_ = false;
    ASSERT_EQ(p_.TmatsStatus(abs_pos_, found_tmats_, false), 
        Ch10Status::OK);
}

TEST_F(Ch10PacketTest, TmatsStatusFirstWorkerCurrTMATsNotFound)
{
    // 0 indicates first worker
    abs_pos_ = 0; 

    // a new worker will always initialize found_tmats to false.
    found_tmats_ = false;
    ASSERT_EQ(p_.TmatsStatus(abs_pos_, found_tmats_, true), Ch10Status::OK);
}

TEST_F(Ch10PacketTest, TmatsStatusFirstWorkerCurrTMATsFound)
{
    // 0 indicates first worker
    abs_pos_ = 0; 

    // TMATS has been previously found by the current worker. That's 
    // ok because multiple multiple tmats packets are possible. 
    found_tmats_ = true;
    ASSERT_EQ(p_.TmatsStatus(abs_pos_, found_tmats_, true), Ch10Status::OK);
}

TEST_F(Ch10PacketTest, TmatsStatusFirstWorkerNotCurrTMATsFound)
{
    // 0 indicates first worker
    abs_pos_ = 0; 

    // TMATS has been previously found by the current worker. That's 
    // ok because multiple multiple tmats packets are possible. 
    found_tmats_ = true;
    ASSERT_EQ(p_.TmatsStatus(abs_pos_, found_tmats_, false), Ch10Status::TMATS_PKT);
}

TEST_F(Ch10PacketTest, TmatsStatusFirstWorkerNotCurrTMATsNotFound)
{
    // 0 indicates first worker
    abs_pos_ = 0; 

    // TMATS has not been previously found by the current worker 
    // and the current packet is not tmats. ch10 requires the first
    // packet to be tmats so this violates the standard.    
    found_tmats_ = false;
    ASSERT_EQ(p_.TmatsStatus(abs_pos_, found_tmats_, false), Ch10Status::TMATS_PKT_ERR);
}