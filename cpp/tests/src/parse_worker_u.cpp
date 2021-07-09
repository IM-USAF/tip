#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parse_worker.h"

class ParseWorkerTest : public ::testing::Test
{
   protected:
    ParseWorker pw_;
    Ch10Context ctx_;
    WorkerConfig worker_cfg_;
    bool result_;

    ParseWorkerTest() : pw_(), ctx_(), worker_cfg_(), result_(false)
    {
        // Initialize worker config
        worker_cfg_.worker_index_ = 0;
        worker_cfg_.start_position_ = 0;
        worker_cfg_.last_position_ = 0;
        worker_cfg_.final_worker_ = false;
        worker_cfg_.append_mode_ = false;
        worker_cfg_.output_file_paths_[Ch10PacketType::MILSTD1553_F1] =
            ManagedPath(std::string("/data/1553"));
        worker_cfg_.output_file_paths_[Ch10PacketType::VIDEO_DATA_F0] =
            ManagedPath(std::string("/data/video"));

        // This map must have at least all of the types defined by default
        // in Ch10Context::CreateDefaultPacketTypeConfig with the exception
        // of COMPUTER_GENERATED_DATA_F1 and TIME_DATA_F1 otherwise tests
        // will fail.
        worker_cfg_.ch10_packet_type_map_[Ch10PacketType::MILSTD1553_F1] = true;
        worker_cfg_.ch10_packet_type_map_[Ch10PacketType::VIDEO_DATA_F0] = false;
        worker_cfg_.ch10_packet_type_map_[Ch10PacketType::ETHERNET_DATA_F0] = false;

        // Initialize Ch10Context
        ctx_.Initialize(worker_cfg_.start_position_, worker_cfg_.worker_index_);
        ctx_.SetSearchingForTDP(!worker_cfg_.append_mode_);
    }
};

TEST_F(ParseWorkerTest, ConfigureContextSetPacketType)
{
    // Ch10Context pkt_type_config_map is configured by the constructor
    // with (currently) four auto-enabled packet types, which are then
    // enabled/disabled by SetPacketTypeConfig which occurs in ConfigureContext.
    result_ = pw_.ConfigureContext(ctx_, worker_cfg_.ch10_packet_type_map_,
                                   worker_cfg_.output_file_paths_);
    EXPECT_TRUE(result_);
    EXPECT_EQ(ctx_.pkt_type_config_map.at(Ch10PacketType::VIDEO_DATA_F0), false);
}

TEST_F(ParseWorkerTest, ConfigureContextCheckConfigurationGood)
{
    result_ = pw_.ConfigureContext(ctx_, worker_cfg_.ch10_packet_type_map_,
                                   worker_cfg_.output_file_paths_);
    EXPECT_TRUE(result_);
    EXPECT_TRUE(ctx_.IsConfigured());
}

TEST_F(ParseWorkerTest, ConfigureContextCheckConfigurationBad)
{
    // Remove the path object associated with 1553, which is enabled.
    // If no path is found for an enabled packet type, the CheckConfiguration
    // function ought to return false.
    EXPECT_EQ(worker_cfg_.output_file_paths_.erase(Ch10PacketType::MILSTD1553_F1), 1);
    result_ = pw_.ConfigureContext(ctx_, worker_cfg_.ch10_packet_type_map_,
                                   worker_cfg_.output_file_paths_);
    EXPECT_FALSE(result_);
    EXPECT_FALSE(ctx_.IsConfigured());
}

TEST_F(ParseWorkerTest, ConfigureContextInitWriters)
{
    // Writers are initially nullptrs
    EXPECT_EQ(ctx_.milstd1553f1_pq_writer, nullptr);
    EXPECT_EQ(ctx_.videof0_pq_writer, nullptr);
    result_ = pw_.ConfigureContext(ctx_, worker_cfg_.ch10_packet_type_map_,
                                   worker_cfg_.output_file_paths_);
    EXPECT_TRUE(result_);

    // Only the milstd1553, which is enabled, ought to have been updated
    // to a non-null pointer.
    EXPECT_TRUE(ctx_.milstd1553f1_pq_writer != nullptr);
}

TEST_F(ParseWorkerTest, ConfigureContextNotIfAlreadyConfigured)
{
    result_ = pw_.ConfigureContext(ctx_, worker_cfg_.ch10_packet_type_map_,
                                   worker_cfg_.output_file_paths_);
    EXPECT_TRUE(result_);
    EXPECT_TRUE(ctx_.IsConfigured());

    worker_cfg_.ch10_packet_type_map_[Ch10PacketType::VIDEO_DATA_F0] = true;
    result_ = pw_.ConfigureContext(ctx_, worker_cfg_.ch10_packet_type_map_,
                                   worker_cfg_.output_file_paths_);
    EXPECT_TRUE(result_);

    // Ch10Context did not update the map.
    EXPECT_FALSE(ctx_.pkt_type_config_map.at(Ch10PacketType::VIDEO_DATA_F0));
}
