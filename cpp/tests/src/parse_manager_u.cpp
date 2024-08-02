#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parse_manager.h"
#include "tmats_data.h"
#include "parser_config_params.h"
#include "ch10_header_format.h"
#include "ch10_1553f1_msg_hdr_format.h"
#include "ch10_tdpf1_hdr_format.h"
#include "parser_paths_mock.h"
#include "parse_manager_mock.h"
#include "worker_config_mock.h"
#include "parser_metadata_mock.h"
#include "managed_path_mock.h"

using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;

class ParseManagerTest : public ::testing::Test
{
   protected:
    ParserConfigParams config;
    ParseManager pm;
    ParseManagerFunctions pmf;
    std::ifstream file;
    std::string line;
    bool result_;
    int retval_;
    std::map<Ch10PacketType, bool> pkt_enabled_map_;
    std::map<Ch10PacketType, ManagedPath> output_dir_map_;
    WorkerConfig worker_config_;

    ParseManagerTest() : result_(false), pm(), pmf(), config(), line(""), worker_config_(),
        file(), retval_(0)
    {}

    bool InitializeParserConfig()
    {
        std::string config_yaml = {
            "ch10_packet_type:\n"
            "  MILSTD1553_FORMAT1: true\n"
            "  VIDEO_FORMAT0 : true\n"
            "parse_chunk_bytes: 500\n"
            "parse_thread_count: 1\n"
            "max_chunk_read_count: 1000\n"
            "worker_offset_wait_ms: 200\n"
            "worker_shift_wait_ms: 200\n"
            "stdout_log_level: info\n"};

        return config.InitializeWithConfigString(config_yaml);
    }
};

TEST_F(ParseManagerTest, IngestUserConfigValidateCalculatedParams)
{
    EXPECT_TRUE(InitializeParserConfig());
    uint64_t file_size = 4e9;
    uint64_t chunk_bytes = 0;
    uint16_t worker_count = 0;
    pmf.IngestUserConfig(config, file_size, chunk_bytes, worker_count);
    EXPECT_EQ(config.parse_chunk_bytes_ * 1e6, chunk_bytes);

    uint16_t expected_worker_count = int(ceil(float(file_size) /
                                              float(chunk_bytes)));
    // Parse architecture was udpated as part of PCM_F1 parsing 
    // development which ensures that the first worker parses
    // TMATs only. Due to this change, and relatively small count
    // of bytes that will be processed by the first worker, and
    // the fact that the expected_worker_count calculation doesn't
    // account for a very small chunk size first worker, another 
    // worker must be added to ensure that the entire ch10 file is 
    // processed/parsed.
    expected_worker_count++;

    EXPECT_EQ(expected_worker_count, worker_count);
}

TEST_F(ParseManagerTest, IngestUserConfigWorkerCountLimited)
{
    EXPECT_TRUE(InitializeParserConfig());
    config.max_chunk_read_count_ = 3;
    uint64_t chunk_bytes = 0;
    uint16_t worker_count = 0;
    uint64_t file_size = 4e9;
    pmf.IngestUserConfig(config, file_size, chunk_bytes, worker_count);
    EXPECT_EQ(config.parse_chunk_bytes_ * 1e6, chunk_bytes);
    EXPECT_EQ(3, worker_count);
}

TEST_F(ParseManagerTest, OpenCh10FileFail)
{
    ManagedPath input_path{"not", "a_real", "directory", "OpenCh10FileTest.txt"};
    std::ifstream input_stream;

    ASSERT_FALSE(pmf.OpenCh10File(input_path, input_stream));
    EXPECT_FALSE(input_stream.is_open());
    EXPECT_FALSE(input_path.is_regular_file());
}

TEST_F(ParseManagerTest, OpenCh10File)
{
    ManagedPath input_path{"OpenCh10FileTest.txt"};
    std::ifstream input_stream;

    std::ofstream out_stream(input_path.string().c_str());
    ASSERT_TRUE(out_stream.is_open());
    out_stream << "here is some text for the file";
    out_stream.close();

    ASSERT_TRUE(pmf.OpenCh10File(input_path, input_stream));
    EXPECT_TRUE(input_stream.is_open());
    EXPECT_TRUE(input_path.is_regular_file());

    input_stream.close();
    EXPECT_TRUE(input_path.remove());
    EXPECT_FALSE(input_path.is_regular_file());
}

TEST_F(ParseManagerTest, MakeWorkUnitsInitializeWorkers)
{
    uint16_t worker_count = 7;
    uint64_t read_size = 483282;
    uint64_t append_read_size = 100000;
    uint64_t total_size = 77149954;
    std::ifstream input_stream;
    MockParserPaths parser_paths;

    ManagedPath bogus_path;
    std::vector<std::map<Ch10PacketType, ManagedPath>> worker_path_vec{
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
    };
    EXPECT_CALL(parser_paths, GetWorkerPathVec()).Times(worker_count)
        .WillRepeatedly(ReturnRef(worker_path_vec));

    std::map<Ch10PacketType, bool> pkt_enabled_map{{Ch10PacketType::MILSTD1553_F1, true}};
    EXPECT_CALL(parser_paths, GetCh10PacketTypeEnabledMap()).Times(worker_count)
        .WillRepeatedly(ReturnRef(pkt_enabled_map));

    std::vector<WorkUnit> work_units;
    ASSERT_TRUE(pmf.MakeWorkUnits(work_units, worker_count, read_size, 
        append_read_size, total_size, input_stream, &parser_paths));
    ASSERT_EQ(worker_count, work_units.size());

    EXPECT_EQ(read_size, work_units.at(0).conf_.read_bytes_);
    EXPECT_EQ(total_size, work_units.at(3).conf_.total_bytes_);
    EXPECT_EQ(6, work_units.at(6).conf_.worker_index_);
}

TEST_F(ParseManagerTest, MakeWorkUnitsInitializeWorkersFail)
{
    uint16_t worker_count = 8;  // too many works for the elements in worker_path_vec
    uint64_t read_size = 483282;
    uint64_t append_read_size = 100000;
    uint64_t total_size = 77149954;
    std::ifstream input_stream;
    MockParserPaths parser_paths;

    ManagedPath bogus_path;
    std::vector<std::map<Ch10PacketType, ManagedPath>> worker_path_vec{
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
        {{Ch10PacketType::MILSTD1553_F1, bogus_path}},
    };
    EXPECT_CALL(parser_paths, GetWorkerPathVec()).Times(worker_count)
        .WillRepeatedly(ReturnRef(worker_path_vec));

    std::map<Ch10PacketType, bool> pkt_enabled_map{{Ch10PacketType::MILSTD1553_F1, true}};
    EXPECT_CALL(parser_paths, GetCh10PacketTypeEnabledMap()).Times(7)
        .WillRepeatedly(ReturnRef(pkt_enabled_map));

    std::vector<WorkUnit> work_units;
    ASSERT_FALSE(pmf.MakeWorkUnits(work_units, worker_count, read_size, 
        append_read_size, total_size, input_stream, &parser_paths));
    ASSERT_EQ(worker_count, work_units.size());
}

TEST_F(ParseManagerTest, ActivateWorkerCheckConfFail)
{
    MockWorkUnit work_unit;
    std::vector<uint16_t> active_workers;
    uint16_t worker_ind = 0;
    bool append_mode = true;
    uint64_t read_pos = 54812589;

    EXPECT_CALL(work_unit, CheckConfiguration(append_mode, read_pos)).WillOnce(Return(false));

    ASSERT_FALSE(pmf.ActivateWorker(&work_unit, active_workers, worker_ind, append_mode, read_pos));
}

TEST_F(ParseManagerTest, ActivateWorker)
{
    MockWorkUnit work_unit;
    std::vector<uint16_t> active_workers;
    uint16_t worker_ind = 7;
    bool append_mode = false;
    uint64_t read_pos = 54812589;

    EXPECT_CALL(work_unit, CheckConfiguration(append_mode, read_pos)).WillOnce(Return(true));
    EXPECT_CALL(work_unit, Activate());

    ASSERT_TRUE(pmf.ActivateWorker(&work_unit, active_workers, worker_ind, append_mode, read_pos));
    ASSERT_EQ(1, active_workers.size());
    EXPECT_EQ(worker_ind, active_workers.at(0));
}

TEST_F(ParseManagerTest, JoinWorker)
{
    MockWorkUnit work_unit;
    std::vector<uint16_t> active_workers{3, 2, 4};
    uint16_t active_worker_ind = 1;

    EXPECT_CALL(work_unit, Join());

    ASSERT_TRUE(pmf.JoinWorker(&work_unit, active_workers, active_worker_ind));
    ASSERT_EQ(2, active_workers.size());
    EXPECT_EQ(4, active_workers.at(1));
}

TEST_F(ParseManagerTest, JoinWorkerRemoveUnavailableIndex)
{
    MockWorkUnit work_unit;
    std::vector<uint16_t> active_workers{2, 3, 5};
    uint16_t active_worker_ind = 3;

    EXPECT_CALL(work_unit, Join());

    ASSERT_FALSE(pmf.JoinWorker(&work_unit, active_workers, active_worker_ind));
    ASSERT_EQ(3, active_workers.size());
}

TEST_F(ParseManagerTest, ActivateInitialThreadAllThreadActivated)
{
    MockWorkUnit work_unit;
    MockParseManagerFunctions mock_pmf;
    std::vector<uint16_t> active_workers{2, 3, 5};
    uint16_t active_worker_ind = 3;
    bool append_mode = false;
    std::chrono::milliseconds worker_wait(10);
    uint64_t read_pos = 383838;
    uint16_t conf_thread_count = 6;
    uint16_t active_thread_count = conf_thread_count;
    bool thread_started = true;

    ASSERT_EQ(EX_OK, pm.ActivateInitialThread(&mock_pmf, append_mode, &work_unit, worker_wait, 
        active_workers, active_worker_ind, read_pos, active_thread_count, conf_thread_count,
        thread_started));
    EXPECT_EQ(false, thread_started);
}

TEST_F(ParseManagerTest, ActivateInitialThreadActivateWorkerFail)
{
    MockWorkUnit work_unit;
    MockParseManagerFunctions mock_pmf;
    std::vector<uint16_t> active_workers{2, 3, 5};
    uint16_t active_worker_ind = 3;
    bool append_mode = false;
    std::chrono::milliseconds worker_wait(10);
    uint64_t init_read_pos = 848190;
    uint64_t read_pos = init_read_pos;
    uint16_t conf_thread_count = 6;
    uint16_t active_thread_count = conf_thread_count - 1;
    bool thread_started = true;

    EXPECT_CALL(mock_pmf, ActivateWorker(&work_unit, active_workers, active_worker_ind, 
        append_mode, read_pos)).WillOnce(Return(false));

    ASSERT_EQ(EX_SOFTWARE, pm.ActivateInitialThread(&mock_pmf, append_mode, &work_unit, worker_wait, 
        active_workers, active_worker_ind, read_pos, active_thread_count, conf_thread_count,
        thread_started));
    EXPECT_EQ(false, thread_started);
}

TEST_F(ParseManagerTest, ActivateInitialThread)
{
    MockWorkUnit work_unit;
    MockParseManagerFunctions mock_pmf;
    std::vector<uint16_t> active_workers{2, 3, 5};
    uint16_t active_worker_ind = 3;
    bool append_mode = false;
    std::chrono::milliseconds worker_wait(10);
    uint64_t init_read_pos = 848190;
    uint64_t read_pos = init_read_pos;
    uint16_t conf_thread_count = 6;
    uint16_t active_thread_count = conf_thread_count - 1;
    bool thread_started = true;

    EXPECT_CALL(mock_pmf, ActivateWorker(&work_unit, active_workers, active_worker_ind, 
        append_mode, read_pos)).WillOnce(Return(true));

    uint64_t read_bytes = 58803;
    EXPECT_CALL(work_unit, GetReadBytes()).WillOnce(ReturnRef(read_bytes));

    ASSERT_EQ(EX_OK, pm.ActivateInitialThread(&mock_pmf, append_mode, &work_unit, worker_wait, 
        active_workers, active_worker_ind, read_pos, active_thread_count, conf_thread_count,
        thread_started));
    EXPECT_EQ(true, thread_started);
    EXPECT_EQ(conf_thread_count, active_thread_count);
    EXPECT_EQ(init_read_pos + read_bytes, read_pos);
}

TEST_F(ParseManagerTest, ActivateAvailableThreadAlreadyStarted)
{
    MockWorkUnit work_unit1;
    MockWorkUnit work_unit2;
    MockWorkUnit work_unit3;
    std::vector<WorkUnit*> work_units{&work_unit1, &work_unit2, &work_unit3};
    MockParseManagerFunctions mock_pmf;
    std::vector<uint16_t> active_workers{2, 3, 5};
    uint16_t active_worker_ind = 3;
    bool append_mode = false;
    std::chrono::milliseconds worker_wait(10);
    uint64_t init_read_pos = 848190;
    uint64_t read_pos = init_read_pos;
    bool thread_started = true;

    ASSERT_EQ(EX_OK, pm.ActivateAvailableThread(&mock_pmf, append_mode, work_units, worker_wait,
        active_workers, active_worker_ind, read_pos, thread_started));
}

TEST_F(ParseManagerTest, ActivateAvailableThreadAllThreadsUnavailable)
{
    MockWorkUnit work_unit1;
    MockWorkUnit work_unit2;
    MockWorkUnit work_unit3;
    MockWorkUnit work_unit4;
    std::vector<WorkUnit*> work_units{&work_unit1, &work_unit2, &work_unit3, &work_unit4};
    MockParseManagerFunctions mock_pmf;
    std::vector<uint16_t> active_workers{0, 2};
    uint16_t active_worker_ind = 3;
    bool append_mode = false;
    std::chrono::milliseconds worker_wait(10);
    uint64_t init_read_pos = 848190;
    uint64_t read_pos = init_read_pos;
    bool thread_started = false;

    EXPECT_CALL(work_unit1, IsComplete()).WillOnce(Return(false));
    EXPECT_CALL(work_unit3, IsComplete()).WillOnce(Return(false));

    ASSERT_EQ(EX_OK, pm.ActivateAvailableThread(&mock_pmf, append_mode, work_units, worker_wait,
        active_workers, active_worker_ind, read_pos, thread_started));
    EXPECT_FALSE(thread_started);
}

TEST_F(ParseManagerTest, ActivateAvailableThreadSecondThreadCompleteActivateWorkerFail)
{
    MockWorkUnit work_unit1;
    MockWorkUnit work_unit2;
    MockWorkUnit work_unit3;
    MockWorkUnit work_unit4;
    std::vector<WorkUnit*> work_units{&work_unit1, &work_unit2, &work_unit3, &work_unit4};
    MockParseManagerFunctions mock_pmf;
    std::vector<uint16_t> active_workers{0, 2};
    uint16_t active_worker_ind = 3;
    bool append_mode = false;
    std::chrono::milliseconds worker_wait(10);
    uint64_t init_read_pos = 848190;
    uint64_t read_pos = init_read_pos;
    bool thread_started = false;

    EXPECT_CALL(work_unit1, IsComplete()).WillOnce(Return(false));
    EXPECT_CALL(work_unit3, IsComplete()).WillOnce(Return(true));

    EXPECT_CALL(mock_pmf, JoinWorker(&work_unit3, active_workers, 1)).WillOnce(Return(true));
    EXPECT_CALL(mock_pmf, ActivateWorker(&work_unit4, active_workers, active_worker_ind,
        append_mode, read_pos)).WillOnce(Return(false));

    ASSERT_EQ(EX_SOFTWARE, pm.ActivateAvailableThread(&mock_pmf, append_mode, work_units, worker_wait,
        active_workers, active_worker_ind, read_pos, thread_started));
    EXPECT_FALSE(thread_started);
}

TEST_F(ParseManagerTest, ActivateAvailableThreadFirstThreadComplete)
{
    MockWorkUnit work_unit1;
    MockWorkUnit work_unit2;
    MockWorkUnit work_unit3;
    MockWorkUnit work_unit4;
    std::vector<WorkUnit*> work_units{&work_unit1, &work_unit2, &work_unit3, &work_unit4};
    MockParseManagerFunctions mock_pmf;
    std::vector<uint16_t> active_workers{0, 2};
    uint16_t active_worker_ind = 3;
    bool append_mode = false;
    std::chrono::milliseconds worker_wait(10);
    uint64_t init_read_pos = 848190;
    uint64_t read_pos = init_read_pos;
    bool thread_started = false;

    EXPECT_CALL(work_unit1, IsComplete()).WillOnce(Return(true));

    EXPECT_CALL(mock_pmf, JoinWorker(&work_unit1, active_workers, 0)).WillOnce(Return(true));
    EXPECT_CALL(mock_pmf, ActivateWorker(&work_unit4, active_workers, active_worker_ind,
        append_mode, read_pos)).WillOnce(Return(true));
    uint64_t read_bytes = 154952;
    EXPECT_CALL(work_unit4, GetReadBytes()).WillOnce(ReturnRef(read_bytes));

    ASSERT_EQ(EX_OK, pm.ActivateAvailableThread(&mock_pmf, append_mode, work_units, worker_wait,
        active_workers, active_worker_ind, read_pos, thread_started));
    EXPECT_TRUE(thread_started);
    EXPECT_EQ(init_read_pos + read_bytes, read_pos);
}

TEST_F(ParseManagerTest, StopThreads)
{
    MockWorkUnit work_unit1;
    MockWorkUnit work_unit2;
    MockWorkUnit work_unit3;
    MockWorkUnit work_unit4;
    std::vector<WorkUnit*> work_units{&work_unit1, &work_unit2, &work_unit3, &work_unit4};
    MockParseManagerFunctions mock_pmf;
    std::vector<uint16_t> active_workers{0, 2};
    int worker_shift_wait = 10;

    ::testing::Sequence seq;
    EXPECT_CALL(work_unit1, IsComplete()).InSequence(seq).WillOnce(Return(false));
    EXPECT_CALL(work_unit3, IsComplete()).InSequence(seq).WillOnce(Return(false));

    EXPECT_CALL(work_unit1, IsComplete()).InSequence(seq).WillOnce(Return(false));
    EXPECT_CALL(work_unit3, IsComplete()).InSequence(seq).WillOnce(Return(true));
    std::vector<uint16_t> new_active_workers1{0};
    EXPECT_CALL(mock_pmf, JoinWorker(&work_unit3, active_workers, 1)).InSequence(seq)
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(new_active_workers1), Return(true)));

    EXPECT_CALL(work_unit1, IsComplete()).InSequence(seq).WillOnce(Return(false));
    EXPECT_CALL(work_unit1, IsComplete()).InSequence(seq).WillOnce(Return(true));
    std::vector<uint16_t> new_active_workers2;
    EXPECT_CALL(mock_pmf, JoinWorker(&work_unit1, new_active_workers1, 0)).InSequence(seq)
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(new_active_workers2), Return(true)));

    ASSERT_TRUE(pm.StopThreads(work_units, active_workers, worker_shift_wait, &mock_pmf));
}

TEST_F(ParseManagerTest, WorkUnitCheckConfigurationFail)
{
    WorkUnit work_unit;
    MockWorkerConfig conf;
    bool append_mode = false;
    uint64_t read_pos = 483811;

    EXPECT_CALL(conf, CheckConfiguration()).WillOnce(Return(false));

    ASSERT_FALSE(work_unit.CheckConfiguration(append_mode, read_pos, &conf));
}

TEST_F(ParseManagerTest, WorkUnitCheckConfigurationPass)
{
    WorkUnit work_unit;
    MockWorkerConfig conf;
    bool append_mode = false;
    uint64_t read_pos = 483811;

    EXPECT_CALL(conf, CheckConfiguration()).WillOnce(Return(true));

    ASSERT_TRUE(work_unit.CheckConfiguration(append_mode, read_pos, &conf));
    EXPECT_EQ(append_mode, conf.append_mode_);
    EXPECT_EQ(read_pos, conf.start_position_);
}

TEST_F(ParseManagerTest, RecordMetadataFail)
{
    MockParserMetadata metadata;
    WorkUnit wu1;
    WorkUnit wu2;
    std::vector<WorkUnit*> work_units{&wu1, &wu2};
    ManagedPath metadata_fname("md.file");

    std::vector<const Ch10Context*> ctx_vec;
    for(std::vector<WorkUnit*>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        ctx_vec.push_back((*it)->ctx_.get());
    
    EXPECT_CALL(metadata, RecordMetadata(metadata_fname, ctx_vec)).WillOnce(Return(false));

    ASSERT_FALSE(pm.RecordMetadata(work_units, &metadata, metadata_fname));
}

TEST_F(ParseManagerTest, RecordMetadataPass)
{
    MockParserMetadata metadata;
    WorkUnit wu1;
    WorkUnit wu2;
    std::vector<WorkUnit*> work_units{&wu1, &wu2};
    ManagedPath metadata_fname("md.file");

    std::vector<const Ch10Context*> ctx_vec;
    for(std::vector<WorkUnit*>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        ctx_vec.push_back((*it)->ctx_.get());
    
    EXPECT_CALL(metadata, RecordMetadata(metadata_fname, ctx_vec)).WillOnce(Return(true));

    ASSERT_TRUE(pm.RecordMetadata(work_units, &metadata, metadata_fname));
}

TEST_F(ParseManagerTest, ConfigureGetFileSizeFail)
{
    MockParserMetadata metadata;
    MockManagedPath ch10_file_path;
    ManagedPath outdir{"test_outdir"};
    ParserConfigParams config;
    MockParseManagerFunctions pmf;
    MockParserPaths parser_paths;
    std::vector<WorkUnit> work_units;
    std::ifstream ch10_stream;

    uint64_t ch10_file_size = 4858584834;
    EXPECT_CALL(ch10_file_path, GetFileSize(_, _)).WillOnce(::testing::DoAll(
        ::testing::SetArgReferee<0>(false), ::testing::SetArgReferee<1>(ch10_file_size)));

    ASSERT_EQ(EX_IOERR, pm.Configure(&ch10_file_path, outdir, config, &pmf, &parser_paths,
        &metadata, work_units, ch10_stream));
}

TEST_F(ParseManagerTest, ConfigureCreateOutputPathsFail)
{
    MockParserMetadata metadata;
    MockManagedPath ch10_file_path;
    ManagedPath outdir{"test_outdir"};
    ParserConfigParams config;
    MockParseManagerFunctions pmf;
    MockParserPaths parser_paths;
    std::vector<WorkUnit> work_units;
    std::ifstream ch10_stream;

    uint64_t ch10_file_size = 4858584834;
    EXPECT_CALL(ch10_file_path, GetFileSize(_, _)).WillOnce(::testing::DoAll(
        ::testing::SetArgReferee<0>(true), ::testing::SetArgReferee<1>(ch10_file_size)));

    uint64_t chunk_bytes = 888582;
    uint16_t worker_count = 12;
    EXPECT_CALL(pmf, IngestUserConfig(config, ch10_file_size, _, _)).
        WillOnce(::testing::DoAll(::testing::SetArgReferee<2>(chunk_bytes), 
        ::testing::SetArgReferee<3>(worker_count)));

    EXPECT_CALL(parser_paths, CreateOutputPaths(ch10_file_path, outdir, 
        config.ch10_packet_enabled_map_, worker_count)).WillOnce(Return(false));

    ASSERT_EQ(EX_CANTCREAT, pm.Configure(&ch10_file_path, outdir, config, &pmf, &parser_paths,
        &metadata, work_units, ch10_stream));
}

TEST_F(ParseManagerTest, ConfigureMakeWorkUnitsFail)
{
    MockParserMetadata metadata;
    MockManagedPath ch10_file_path;
    ManagedPath outdir{"test_outdir"};
    ParserConfigParams config;
    MockParseManagerFunctions pmf;
    MockParserPaths parser_paths;
    std::vector<WorkUnit> work_units;
    std::ifstream ch10_stream;

    uint64_t ch10_file_size = 4858584834;
    EXPECT_CALL(ch10_file_path, GetFileSize(_, _)).WillOnce(::testing::DoAll(
        ::testing::SetArgReferee<0>(true), ::testing::SetArgReferee<1>(ch10_file_size)));

    uint64_t chunk_bytes = 888582;
    uint16_t worker_count = 12;
    EXPECT_CALL(pmf, IngestUserConfig(config, ch10_file_size, _, _))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<2>(chunk_bytes), 
        ::testing::SetArgReferee<3>(worker_count)));

    EXPECT_CALL(parser_paths, CreateOutputPaths(ch10_file_path, outdir, 
        config.ch10_packet_enabled_map_, worker_count)).WillOnce(Return(true));

    EXPECT_CALL(pmf, MakeWorkUnits(::testing::Ref(work_units), worker_count, chunk_bytes, 
        ParseManager::GetAppendChunkSizeBytes(), ch10_file_size, ::testing::Ref(ch10_stream), 
        &parser_paths)).WillOnce(Return(false));

    ASSERT_EQ(EX_SOFTWARE, pm.Configure(&ch10_file_path, outdir, config, &pmf, &parser_paths,
        &metadata, work_units, ch10_stream));
}

TEST_F(ParseManagerTest, ConfigureInitializeFail)
{
    MockParserMetadata metadata;
    MockManagedPath ch10_file_path;
    ManagedPath outdir{"test_outdir"};
    ParserConfigParams config;
    MockParseManagerFunctions pmf;
    MockParserPaths parser_paths;
    std::vector<WorkUnit> work_units;
    std::ifstream ch10_stream;

    uint64_t ch10_file_size = 4858584834;
    EXPECT_CALL(ch10_file_path, GetFileSize(_, _)).WillOnce(::testing::DoAll(
        ::testing::SetArgReferee<0>(true), ::testing::SetArgReferee<1>(ch10_file_size)));

    uint64_t chunk_bytes = 888582;
    uint16_t worker_count = 12;
    EXPECT_CALL(pmf, IngestUserConfig(config, ch10_file_size, _, _))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<2>(chunk_bytes), 
        ::testing::SetArgReferee<3>(worker_count)));

    EXPECT_CALL(parser_paths, CreateOutputPaths(ch10_file_path, outdir, 
        config.ch10_packet_enabled_map_, worker_count)).WillOnce(Return(true));

    EXPECT_CALL(pmf, MakeWorkUnits(::testing::Ref(work_units), worker_count, chunk_bytes, 
        ParseManager::GetAppendChunkSizeBytes(), ch10_file_size, ::testing::Ref(ch10_stream), 
        &parser_paths)).WillOnce(Return(true));

    EXPECT_CALL(metadata, Initialize(ch10_file_path, config, ::testing::Ref(parser_paths)))
        .WillOnce(Return(EX_IOERR));

    ASSERT_EQ(EX_IOERR, pm.Configure(&ch10_file_path, outdir, config, &pmf, &parser_paths,
        &metadata, work_units, ch10_stream));
}

TEST_F(ParseManagerTest, ConfigureOpenCh10FileFail)
{
    MockParserMetadata metadata;
    MockManagedPath ch10_file_path;
    ManagedPath outdir{"test_outdir"};
    ParserConfigParams config;
    MockParseManagerFunctions pmf;
    MockParserPaths parser_paths;
    std::vector<WorkUnit> work_units;
    std::ifstream ch10_stream;

    uint64_t ch10_file_size = 4858584834;
    EXPECT_CALL(ch10_file_path, GetFileSize(_, _)).WillOnce(::testing::DoAll(
        ::testing::SetArgReferee<0>(true), ::testing::SetArgReferee<1>(ch10_file_size)));

    uint64_t chunk_bytes = 888582;
    uint16_t worker_count = 12;
    EXPECT_CALL(pmf, IngestUserConfig(config, ch10_file_size, _, _))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<2>(chunk_bytes), 
        ::testing::SetArgReferee<3>(worker_count)));

    EXPECT_CALL(parser_paths, CreateOutputPaths(ch10_file_path, outdir, 
        config.ch10_packet_enabled_map_, worker_count)).WillOnce(Return(true));

    EXPECT_CALL(pmf, MakeWorkUnits(::testing::Ref(work_units), worker_count, chunk_bytes, 
        ParseManager::GetAppendChunkSizeBytes(), ch10_file_size, ::testing::Ref(ch10_stream), 
        &parser_paths)).WillOnce(Return(true));

    EXPECT_CALL(metadata, Initialize(ch10_file_path, config, ::testing::Ref(parser_paths)))
        .WillOnce(Return(EX_OK));

    EXPECT_CALL(pmf, OpenCh10File(ch10_file_path, ::testing::Ref(ch10_stream)))
        .WillOnce(Return(false));

    ASSERT_EQ(EX_IOERR, pm.Configure(&ch10_file_path, outdir, config, &pmf, &parser_paths,
        &metadata, work_units, ch10_stream));
}

TEST_F(ParseManagerTest, Configure)
{
    MockParserMetadata metadata;
    MockManagedPath ch10_file_path;
    ManagedPath outdir{"test_outdir"};
    ParserConfigParams config;
    MockParseManagerFunctions pmf;
    MockParserPaths parser_paths;
    std::vector<WorkUnit> work_units;
    std::ifstream ch10_stream;

    uint64_t ch10_file_size = 4858584834;
    EXPECT_CALL(ch10_file_path, GetFileSize(_, _)).WillOnce(::testing::DoAll(
        ::testing::SetArgReferee<0>(true), ::testing::SetArgReferee<1>(ch10_file_size)));

    uint64_t chunk_bytes = 888582;
    uint16_t worker_count = 12;
    EXPECT_CALL(pmf, IngestUserConfig(config, ch10_file_size, _, _))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<2>(chunk_bytes), 
        ::testing::SetArgReferee<3>(worker_count)));

    EXPECT_CALL(parser_paths, CreateOutputPaths(ch10_file_path, outdir, 
        config.ch10_packet_enabled_map_, worker_count)).WillOnce(Return(true));

    EXPECT_CALL(pmf, MakeWorkUnits(::testing::Ref(work_units), worker_count, chunk_bytes, 
        ParseManager::GetAppendChunkSizeBytes(), ch10_file_size, ::testing::Ref(ch10_stream), 
        &parser_paths)).WillOnce(Return(true));

    EXPECT_CALL(metadata, Initialize(ch10_file_path, config, ::testing::Ref(parser_paths)))
        .WillOnce(Return(EX_OK));

    EXPECT_CALL(pmf, OpenCh10File(ch10_file_path, ::testing::Ref(ch10_stream)))
        .WillOnce(Return(true));

    ASSERT_EQ(EX_OK, pm.Configure(&ch10_file_path, outdir, config, &pmf, &parser_paths,
        &metadata, work_units, ch10_stream));
}

TEST_F(ParseManagerTest, ParseCh10InitialStartThreadsFail)
{
    size_t work_unit_count = 5;
    std::vector<WorkUnit> work_units(work_unit_count);

    std::vector<WorkUnit*> work_unit_ptrs;
    for (std::vector<WorkUnit>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        work_unit_ptrs.push_back(&(*it));

    ParserConfigParams config;
    MockParseManager pm;
    ParseManagerFunctions pmf;
    bool append = false;

    std::vector<uint16_t> active_workers1;
    EXPECT_CALL(pm, StartThreads(append, _, static_cast<uint16_t>(work_units.size()), config, work_unit_ptrs, &pmf))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers1), Return(70)));

    ASSERT_EQ(EX_SOFTWARE, ParseCh10(work_unit_ptrs, &pmf, &pm, config));
}

TEST_F(ParseManagerTest, ParseCh10InitialStopThreadsFail)
{
    size_t work_unit_count = 5;
    std::vector<WorkUnit> work_units(work_unit_count);

    std::vector<WorkUnit*> work_unit_ptrs;
    for (std::vector<WorkUnit>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        work_unit_ptrs.push_back(&(*it));

    ParserConfigParams config;
    config.worker_offset_wait_ms_ = 200;
    config.worker_shift_wait_ms_ = 100;
    MockParseManager pm;
    ParseManagerFunctions pmf;
    bool append = false;

    std::vector<uint16_t> active_workers1{3, 4};
    EXPECT_CALL(pm, StartThreads(append, _, static_cast<uint16_t>(work_units.size()), config, work_unit_ptrs, &pmf))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers1), Return(0)));

    std::vector<uint16_t> active_workers2;
    EXPECT_CALL(pm, StopThreads(work_unit_ptrs, active_workers1, config.worker_shift_wait_ms_,
        &pmf)).WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers2), Return(false)));

    ASSERT_EQ(EX_SOFTWARE, ParseCh10(work_unit_ptrs, &pmf, &pm, config));
}

TEST_F(ParseManagerTest, ParseCh10SingleThreadEarlyReturn)
{
    size_t work_unit_count = 1;
    std::vector<WorkUnit> work_units(work_unit_count);

    std::vector<WorkUnit*> work_unit_ptrs;
    for (std::vector<WorkUnit>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        work_unit_ptrs.push_back(&(*it));

    ParserConfigParams config;
    config.worker_offset_wait_ms_ = 200;
    config.worker_shift_wait_ms_ = 100;
    MockParseManager pm;
    ParseManagerFunctions pmf;
    bool append = false;

    std::vector<uint16_t> active_workers1{0};
    EXPECT_CALL(pm, StartThreads(append, _, static_cast<uint16_t>(work_units.size()), config, work_unit_ptrs, &pmf))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers1), Return(0)));

    std::vector<uint16_t> active_workers2;
    EXPECT_CALL(pm, StopThreads(work_unit_ptrs, active_workers1, config.worker_shift_wait_ms_,
        &pmf)).WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers2), Return(true)));

    ASSERT_EQ(EX_OK, ParseCh10(work_unit_ptrs, &pmf, &pm, config));
}

TEST_F(ParseManagerTest, ParseCh10FinalStartThreadsFail)
{
    size_t work_unit_count = 5;
    std::vector<WorkUnit> work_units(work_unit_count);

    std::vector<WorkUnit*> work_unit_ptrs;
    for (std::vector<WorkUnit>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        work_unit_ptrs.push_back(&(*it));

    ParserConfigParams config;
    config.worker_offset_wait_ms_ = 200;
    config.worker_shift_wait_ms_ = 100;
    MockParseManager pm;
    ParseManagerFunctions pmf;
    bool append = false;

    std::vector<uint16_t> active_workers1{3, 4};
    EXPECT_CALL(pm, StartThreads(append, _, static_cast<uint16_t>(work_units.size()), config, work_unit_ptrs, &pmf))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers1), Return(0)));

    std::vector<uint16_t> active_workers2;
    EXPECT_CALL(pm, StopThreads(work_unit_ptrs, active_workers1, config.worker_shift_wait_ms_,
        &pmf)).WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers2), Return(true)));

    append = true;
    std::vector<uint16_t> active_workers3{2, 3};
    EXPECT_CALL(pm, StartThreads(append, active_workers2, static_cast<uint16_t>(work_units.size()-1), config, 
        work_unit_ptrs, &pmf)).WillOnce(::testing::DoAll(
            ::testing::SetArgReferee<1>(active_workers3), Return(70)));

    ASSERT_EQ(EX_SOFTWARE, ParseCh10(work_unit_ptrs, &pmf, &pm, config));
}

TEST_F(ParseManagerTest, ParseCh10FinalStopThreadsFail)
{
    size_t work_unit_count = 5;
    std::vector<WorkUnit> work_units(work_unit_count);

    std::vector<WorkUnit*> work_unit_ptrs;
    for (std::vector<WorkUnit>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        work_unit_ptrs.push_back(&(*it));

    ParserConfigParams config;
    config.worker_offset_wait_ms_ = 200;
    config.worker_shift_wait_ms_ = 100;
    MockParseManager pm;
    ParseManagerFunctions pmf;
    bool append = false;

    std::vector<uint16_t> active_workers1{3, 4};
    EXPECT_CALL(pm, StartThreads(append, _, static_cast<uint16_t>(work_units.size()), config, work_unit_ptrs, &pmf))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers1), Return(0)));

    std::vector<uint16_t> active_workers2;
    EXPECT_CALL(pm, StopThreads(work_unit_ptrs, active_workers1, config.worker_shift_wait_ms_,
        &pmf)).WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers2), Return(true)));

    append = true;
    std::vector<uint16_t> active_workers3{2, 3};
    EXPECT_CALL(pm, StartThreads(append, active_workers2, static_cast<uint16_t>(work_units.size()-1), config, 
        work_unit_ptrs, &pmf)).WillOnce(::testing::DoAll(
            ::testing::SetArgReferee<1>(active_workers3), Return(0)));

    std::vector<uint16_t> active_workers4;
    EXPECT_CALL(pm, StopThreads(work_unit_ptrs, active_workers3, config.worker_shift_wait_ms_,
        &pmf)).WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers4), Return(false)));

    ASSERT_EQ(EX_SOFTWARE, ParseCh10(work_unit_ptrs, &pmf, &pm, config));
}

TEST_F(ParseManagerTest, ParseCh10)
{
    size_t work_unit_count = 5;
    std::vector<WorkUnit> work_units(work_unit_count);

    std::vector<WorkUnit*> work_unit_ptrs;
    for (std::vector<WorkUnit>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        work_unit_ptrs.push_back(&(*it));

    ParserConfigParams config;
    config.worker_offset_wait_ms_ = 200;
    config.worker_shift_wait_ms_ = 100;
    MockParseManager pm;
    ParseManagerFunctions pmf;
    bool append = false;

    std::vector<uint16_t> active_workers1{3, 4};
    EXPECT_CALL(pm, StartThreads(append, _, static_cast<uint16_t>(work_units.size()), config, work_unit_ptrs, &pmf))
        .WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers1), Return(0)));

    std::vector<uint16_t> active_workers2;
    EXPECT_CALL(pm, StopThreads(work_unit_ptrs, active_workers1, config.worker_shift_wait_ms_,
        &pmf)).WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers2), Return(true)));

    append = true;
    std::vector<uint16_t> active_workers3{2, 3};
    EXPECT_CALL(pm, StartThreads(append, active_workers2, static_cast<uint16_t>(work_units.size()-1), config, 
        work_unit_ptrs, &pmf)).WillOnce(::testing::DoAll(
            ::testing::SetArgReferee<1>(active_workers3), Return(0)));

    std::vector<uint16_t> active_workers4;
    EXPECT_CALL(pm, StopThreads(work_unit_ptrs, active_workers3, config.worker_shift_wait_ms_,
        &pmf)).WillOnce(::testing::DoAll(::testing::SetArgReferee<1>(active_workers4), Return(true)));

    ASSERT_EQ(EX_OK, ParseCh10(work_unit_ptrs, &pmf, &pm, config));
}