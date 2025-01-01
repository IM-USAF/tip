#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "worker_config.h"
#include "binbuff_mock.h"
#include "parser_paths_mock.h"

using ::testing::ReturnRef;

class WorkerConfigTest : public ::testing::Test
{
   protected:
    WorkerConfig wc_;
    // WorkerConfig twc_;
    MockBinBuff mock_bb_;

    WorkerConfigTest() : wc_(), mock_bb_()//, twc_()
    {}
};

TEST_F(WorkerConfigTest, CheckNonAppendModeConfigIndexTooHigh)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_;
    ASSERT_FALSE(wc_.CheckNonAppendModeConfig());
}

TEST_F(WorkerConfigTest, CheckNonAppendModeConfigFinalWorkerReadFail)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 1;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;
    wc_.total_bytes_ = 4983828;
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_mode_ = true;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.start_position_, wc_.read_bytes_))
        .WillOnce(::testing::Return(UINT64_MAX));

    ASSERT_FALSE(wc_.CheckNonAppendModeConfig());
    EXPECT_EQ(true, wc_.final_worker_);
    EXPECT_FALSE(wc_.append_mode_);
}

TEST_F(WorkerConfigTest, CheckNonAppendModeConfigReadFail)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 3;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;
    wc_.total_bytes_ = 4983828;
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_mode_ = true;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.start_position_, wc_.read_bytes_))
        .WillOnce(::testing::Return(UINT64_MAX));

    ASSERT_FALSE(wc_.CheckNonAppendModeConfig());
    EXPECT_EQ(false, wc_.final_worker_);
    EXPECT_FALSE(wc_.append_mode_);
}

TEST_F(WorkerConfigTest, CheckNonAppendModeConfigIncorrectReadSize)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 3;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;
    wc_.total_bytes_ = 4983828;
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_mode_ = true;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.start_position_, wc_.read_bytes_))
        .WillOnce(::testing::Return(wc_.read_bytes_ + 1000));

    ASSERT_FALSE(wc_.CheckNonAppendModeConfig());
    EXPECT_EQ(false, wc_.final_worker_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.read_bytes_ + 1000);
    EXPECT_FALSE(wc_.append_mode_);
}

TEST_F(WorkerConfigTest, CheckNonAppendModeConfigIncorrectReadSizeLastWorker)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 1;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 4983828;
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_mode_ = true;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.start_position_, wc_.read_bytes_))
        .WillOnce(::testing::Return(wc_.read_bytes_ - 1000));

    ASSERT_TRUE(wc_.CheckNonAppendModeConfig());
    EXPECT_EQ(true, wc_.final_worker_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.read_bytes_ - 1000);
    EXPECT_FALSE(wc_.append_mode_);
}

TEST_F(WorkerConfigTest, CheckNonAppendModeConfigReadSizeGreaterTotalCh10Size)
{
    wc_.worker_index_ = 0;
    wc_.total_worker_count_ = wc_.worker_index_ + 3;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 83828;
    wc_.start_position_ = 0;
    wc_.read_bytes_ = 100000;
    wc_.append_mode_ = true;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.start_position_, wc_.read_bytes_))
        .WillOnce(::testing::Return(wc_.total_bytes_));

    ASSERT_TRUE(wc_.CheckNonAppendModeConfig());
    EXPECT_EQ(true, wc_.final_worker_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.total_bytes_);
    EXPECT_FALSE(wc_.append_mode_);
}

TEST_F(WorkerConfigTest, CheckNonAppendModeConfig)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 3;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 4983828;
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_mode_ = true;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.start_position_, wc_.read_bytes_))
        .WillOnce(::testing::Return(wc_.read_bytes_));

    ASSERT_TRUE(wc_.CheckNonAppendModeConfig());
    EXPECT_EQ(false, wc_.final_worker_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.read_bytes_);
    EXPECT_FALSE(wc_.append_mode_);
}

TEST_F(WorkerConfigTest, CheckAppendModeConfigReadFail)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 1;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 4983828;
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_read_bytes_ = 2000;
    wc_.last_position_ = 38481;
    wc_.append_mode_ = false;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.last_position_, wc_.append_read_bytes_))
        .WillOnce(::testing::Return(UINT64_MAX));

    ASSERT_FALSE(wc_.CheckAppendModeConfig());
    EXPECT_TRUE(wc_.append_mode_);
    EXPECT_EQ(wc_.start_position_, wc_.last_position_);
}

TEST_F(WorkerConfigTest, CheckAppendModeConfigIncorrectReadBytes)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 1;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 4983828;
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_read_bytes_ = 2000;
    wc_.last_position_ = 38481;
    wc_.append_mode_ = false;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.last_position_, wc_.append_read_bytes_))
        .WillOnce(::testing::Return(wc_.append_read_bytes_ + 1848));

    ASSERT_FALSE(wc_.CheckAppendModeConfig());
    EXPECT_TRUE(wc_.append_mode_);
    EXPECT_EQ(wc_.start_position_, wc_.last_position_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.append_read_bytes_ + 1848);
}

TEST_F(WorkerConfigTest, CheckAppendModeConfigIncorrectReadBytesEOF)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 1;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 3828;  // total_bytes < (start_position + append_read_bytes))
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.last_position_ = 38481;
    wc_.append_mode_ = false;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.last_position_, wc_.append_read_bytes_))
        .WillOnce(::testing::Return(wc_.append_read_bytes_ - 1848));

    ASSERT_TRUE(wc_.CheckAppendModeConfig());
    EXPECT_TRUE(wc_.append_mode_);
    EXPECT_EQ(wc_.start_position_, wc_.last_position_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.append_read_bytes_ - 1848);
    EXPECT_TRUE((wc_.start_position_ + wc_.append_read_bytes_) > wc_.total_bytes_);
}

TEST_F(WorkerConfigTest, CheckAppendModeConfig)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 1;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 3828;  // total_bytes < (start_position + append_read_bytes))
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.last_position_ = 38481;
    wc_.append_mode_ = false;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.last_position_, wc_.append_read_bytes_))
        .WillOnce(::testing::Return(wc_.append_read_bytes_));

    ASSERT_TRUE(wc_.CheckAppendModeConfig());
    EXPECT_TRUE(wc_.append_mode_);
    EXPECT_EQ(wc_.start_position_, wc_.last_position_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.append_read_bytes_);
}

TEST_F(WorkerConfigTest, CheckConfigurationAppendModePass)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 1;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 3828000;  // total_bytes < (start_position + append_read_bytes))
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_read_bytes_ = 2000;
    wc_.last_position_ = 38481;
    wc_.append_mode_ = true;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.last_position_, wc_.append_read_bytes_))
        .WillOnce(::testing::Return(wc_.append_read_bytes_));

    ASSERT_TRUE(wc_.CheckConfiguration());
    EXPECT_TRUE(wc_.append_mode_);
    EXPECT_EQ(wc_.start_position_, wc_.last_position_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.append_read_bytes_);
}

TEST_F(WorkerConfigTest, CheckConfigurationAppendModeFail)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 1;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 3828000;  // total_bytes < (start_position + append_read_bytes))
    wc_.start_position_ = 548483;
    wc_.append_read_bytes_ = 2000;
    wc_.read_bytes_ = 2300;
    wc_.last_position_ = 38481;
    wc_.append_mode_ = true;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.last_position_, wc_.append_read_bytes_))
        .WillOnce(::testing::Return(wc_.append_read_bytes_ + 38281));

    ASSERT_FALSE(wc_.CheckConfiguration());
    EXPECT_TRUE(wc_.append_mode_);
    EXPECT_EQ(wc_.start_position_, wc_.last_position_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.append_read_bytes_ + 38281);
}

TEST_F(WorkerConfigTest, CheckConfigurationNonAppendModePass)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 3;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 4983828;
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_mode_ = false;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.start_position_, wc_.read_bytes_))
        .WillOnce(::testing::Return(wc_.read_bytes_));

    ASSERT_TRUE(wc_.CheckConfiguration());
    EXPECT_EQ(false, wc_.final_worker_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.read_bytes_);
    EXPECT_FALSE(wc_.append_mode_);
}

TEST_F(WorkerConfigTest, CheckConfigurationNonAppendModeFail)
{
    wc_.worker_index_ = 13;
    wc_.total_worker_count_ = wc_.worker_index_ + 3;
    wc_.final_worker_ = false;
    wc_.bb_ = &mock_bb_;
    std::ifstream input_stream;
    wc_.input_stream_ = &input_stream;

    wc_.total_bytes_ = 4983828;
    wc_.start_position_ = 548483;
    wc_.read_bytes_ = 2300;
    wc_.append_mode_ = false;
    
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(*wc_.input_stream_), 
        wc_.total_bytes_, wc_.start_position_, wc_.read_bytes_))
        .WillOnce(::testing::Return(wc_.read_bytes_ - 100));

    ASSERT_FALSE(wc_.CheckConfiguration());
    EXPECT_EQ(false, wc_.final_worker_);
    EXPECT_EQ(wc_.actual_read_bytes_, wc_.read_bytes_ - 100);
    EXPECT_FALSE(wc_.append_mode_);
}

TEST_F(WorkerConfigTest, InitializeWorkerConfigWorkerPathVecInsufficientSize)
{
    uint16_t worker_count = 10;
    uint16_t worker_index = 0;
    uint64_t read_size = 483282;
    uint64_t append_read_size = 100000;
    uint64_t total_size = 77149954;
    std::ifstream input_stream;
    MockParserPaths parser_paths;
    WorkerConfig conf;

    std::vector<std::map<Ch10PacketType, ManagedPath>> worker_path_vec;
    EXPECT_CALL(parser_paths, GetWorkerPathVec()).WillOnce(ReturnRef(worker_path_vec));

    ASSERT_FALSE(conf.Initialize(worker_count, worker_index, read_size,
        append_read_size, total_size, input_stream, &parser_paths));
}

TEST_F(WorkerConfigTest, Initialize)
{
    uint16_t worker_count = 10;
    uint16_t worker_index = 0;
    uint64_t read_size = 483282;
    uint64_t append_read_size = 100000;
    uint64_t total_size = 77149954;
    std::ifstream input_stream;
    MockParserPaths parser_paths;
    WorkerConfig conf;

    ManagedPath bogus_path;
    std::map<Ch10PacketType, ManagedPath> worker_path_elem{
        {Ch10PacketType::MILSTD1553_F1, bogus_path}};
    std::vector<std::map<Ch10PacketType, ManagedPath>> worker_path_vec{worker_path_elem};
    EXPECT_CALL(parser_paths, GetWorkerPathVec()).WillOnce(ReturnRef(worker_path_vec));

    std::map<Ch10PacketType, bool> pkt_enabled_map{{Ch10PacketType::MILSTD1553_F1, true}};
    EXPECT_CALL(parser_paths, GetCh10PacketTypeEnabledMap()).WillOnce(ReturnRef(pkt_enabled_map));

    ASSERT_TRUE(conf.Initialize(worker_count, worker_index, read_size,
        append_read_size, total_size, input_stream, &parser_paths));

    EXPECT_EQ(worker_index, conf.worker_index_);
    EXPECT_EQ(worker_count, conf.total_worker_count_);
    EXPECT_EQ(read_size, conf.read_bytes_);
    EXPECT_EQ(append_read_size, conf.append_read_bytes_);
    EXPECT_EQ(total_size, conf.total_bytes_);
    EXPECT_EQ(&input_stream, conf.input_stream_);
    EXPECT_EQ(worker_path_elem.size(), conf.output_file_paths_.size());
    EXPECT_EQ(worker_path_elem.at(Ch10PacketType::MILSTD1553_F1), 
        conf.output_file_paths_.at(Ch10PacketType::MILSTD1553_F1));
    EXPECT_THAT(pkt_enabled_map, ::testing::ContainerEq(conf.ch10_packet_type_map_));
}