#include "worker_config.h"

WorkerConfig::WorkerConfig() : worker_index_(0), start_position_(0), last_position_(0), 
        final_worker_(false), append_mode_(false), bb_unique_(std::make_unique<BinBuff>()), 
        total_worker_count_(0),
        read_bytes_(0),
        append_read_bytes_(0),
        total_bytes_(0),
        input_stream_(nullptr),
        actual_read_bytes_(0)
    {
        bb_ = bb_unique_.get();
    }



bool WorkerConfig::CheckConfiguration()
{
    if (this->append_mode_)
    {
        if (!CheckAppendModeConfig())
        {
            spdlog::get("pm_logger")->warn(
                "ActivateWorker: ConfigureAppendWorker failed during "
                "initial thread loading");
            return false;
        }
    }
    else
    {
        if (!CheckNonAppendModeConfig())
        {
            spdlog::get("pm_logger")->warn(
                "ActivateWorker: WorkerConfig failed during "
                "initial thread loading");
            return false;
        }
    }

    // worker_thread = std::thread(std::ref(*parse_worker_ptr), std::ref(worker_config), ctx.get());

    return true;

}

bool WorkerConfig::CheckNonAppendModeConfig()
{
    // Check for invalid worker_index
    if (this->worker_index_ > this->total_worker_count_ - 1)
    {
        spdlog::get("pm_logger")->warn("WorkerConfig: worker_index ({:d}) > worker_count ({:d}) - 1",
            this->worker_index_, this->total_worker_count_);
        return false;
    }

    // Final worker is the last in order of index and chunks to be parsed
    // from the ch10. As such there is no append mode for this worker, so
    // it will need to know in order to close the file writers prior
    // to returning.
    this->final_worker_ = false;
    if (this->worker_index_ == this->total_worker_count_ - 1)
    {
        this->final_worker_ = true;
    }

    this->append_mode_ = false;

    spdlog::get("pm_logger")->debug("WorkerConfig {:d}: start = {:d}, read size = {:d}", 
        this->worker_index_, this->start_position_, this->read_bytes_);

    this->actual_read_bytes_ = this->bb_->Initialize(*(this->input_stream_),
                                               this->total_bytes_, 
                                               this->start_position_, this->read_bytes_);

    if (this->actual_read_bytes_ == UINT64_MAX)
        return false;

    if (this->actual_read_bytes_ != this->read_bytes_)
    {
        spdlog::get("pm_logger")->debug(
            "WorkerConfig: worker {:d} actual read size ({:d}) "
            "not equal to requested read size ({:d})",
            this->worker_index_, this->actual_read_bytes_, this->read_bytes_);

        // If this is the first worker and the configured read
        // size is greater than the total ch10 file size then there is no
        // expectation that the actual read size is equal to requested read
        // size.
        if((this->worker_index_ == 0) && 
            (this->actual_read_bytes_ == this->total_bytes_))
        {
            spdlog::get("pm_logger")->debug("WorkerConfig: First worker reached EOF OK");
            this->final_worker_ = true;
            return true;
        }

        // Alternately, if the last worker is being configured, it will undoubtedly reach the
        // EOF and this will occur, which is not an error.
        if ((this->worker_index_ == this->total_worker_count_ - 1) && 
            (this->actual_read_bytes_ < this->read_bytes_))
        {
            spdlog::get("pm_logger")->debug("WorkerConfig: Last worker reached EOF OK");
            return true;
        }
        return false;
    }
    return true;
}

bool WorkerConfig::CheckAppendModeConfig()
{
    this->start_position_ = this->last_position_;
    this->append_mode_ = true;

    spdlog::get("pm_logger")->debug(
        "ConfigureAppendWorker: worker {:d} initializing buffer at position "
        "{:d}, reading {:d} bytes from file with total size {:d}",
        this->worker_index_, this->start_position_, 
        this->append_read_bytes_, this->total_bytes_);

    this->actual_read_bytes_ = this->bb_->Initialize(*this->input_stream_,
                                               this->total_bytes_, 
                                               this->start_position_, 
                                               this->append_read_bytes_);

    if (this->actual_read_bytes_ == UINT64_MAX)
        return false;

    // Append mode workers ought to never read to the end of the file.
    // The final append mode worker parses the data beginning at the location
    // where the second to last first-pass worker stopped parsing due to
    // an incomplete packet in the buffer. If a read failure occurs, indicated
    // by the following logic, during configuration of an append-mode worker,
    // then an error has occurred.
    if (this->actual_read_bytes_ != this->append_read_bytes_)
    {
        // It's possible that the last worker only read, for example,
        // 5MB because of the way the ch10 is chunked. Say the second-to-last
        // worker parsed up to last megabyte or so of the chunk it was given.
        // (Typically it parses up until the last few bytes or kilobytes.)
        // Then the append worker for the second-to-last worker will try to
        // read the current default append_read_size = 100MB. However, in this
        // example there is only 1MB + 5 MB left in the file, so the read
        // size is not what is requested. Allow for this to happen without
        // indicating an error.
        if (this->start_position_ + this->append_read_bytes_ > this->total_bytes_)
            return true;

        spdlog::get("pm_logger")->warn(
            "ConfigureAppendWorker: worker {:d} actual read size ({:d}) not "
            "equal to requested read size ({:d})",
            this->worker_index_, this->actual_read_bytes_, this->append_read_bytes_);
        return false;
    }

    return true;
}

bool WorkerConfig::Initialize(const uint16_t& worker_count, const uint16_t& worker_index, 
    const uint64_t& read_size, const uint32_t& append_read_size, const uint64_t& total_size, 
    std::ifstream& ch10_input_stream, const ParserPaths* parser_paths)
{
    worker_index_ = worker_index;
    total_worker_count_ = worker_count;
    read_bytes_ = read_size;
    append_read_bytes_ = append_read_size;
    total_bytes_ = total_size;
    input_stream_ = &ch10_input_stream;
    std::vector<std::map<Ch10PacketType, ManagedPath>> worker_path_vec = 
        parser_paths->GetWorkerPathVec();
    if (worker_path_vec.size() < (worker_index + 1))
    {
        spdlog::get("pm_logger")->warn("Worker paths vector size ({:d})is not sufficient "
            "for worker_index ({:d})", worker_path_vec.size(), worker_index);
        return false;
    }
    output_file_paths_ = worker_path_vec.at(worker_index);
    ch10_packet_type_map_ = parser_paths->GetCh10PacketTypeEnabledMap();

    return true;
}
