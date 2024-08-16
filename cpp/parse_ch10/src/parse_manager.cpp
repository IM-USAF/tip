// parse_manager.cpp

#include "parse_manager.h"

ParseManager::ParseManager() : retcode_(0) {}

ParseManager::~ParseManager()
{}

const std::string ParseManager::metadata_filename_ = "_metadata.yaml";
const uint32_t ParseManager::append_chunk_size_bytes_ = 100000000;

uint32_t ParseManager::GetAppendChunkSizeBytes()
{
    return append_chunk_size_bytes_;
}

std::string ParseManager::GetMetadataFilename()
{
    return metadata_filename_;
}


int Parse(ManagedPath ch10_path, ManagedPath out_dir, const ParserConfigParams& config)
{
    ParseManager pm;
    ParseManagerFunctions pmf;
    ParserPaths parser_paths;
    ParserMetadata metadata;
    std::ifstream ch10_input_stream;
    std::vector<WorkUnit> work_units;
    int retcode = 0;

    if((retcode = pm.Configure(&ch10_path, out_dir, config, &pmf, &parser_paths, &metadata, 
        work_units, ch10_input_stream)) != 0)
    {
        spdlog::get("pm_logger")->error("Parse error: ParseManager::Configure failure");
        ch10_input_stream.close();
        return retcode;
    }

    std::vector<WorkUnit*> work_unit_ptrs;
    for (std::vector<WorkUnit>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        work_unit_ptrs.push_back(&(*it));

    if((retcode = ParseCh10(work_unit_ptrs, &pmf, &pm, config)) != 0)
    {
        spdlog::get("pm_logger")->error("Parse error: ParseCh10 failure");
        ch10_input_stream.close();
        return retcode;
    }

    ManagedPath metadata_fname(pm.metadata_filename_);
    if((retcode = pm.RecordMetadata(work_unit_ptrs, &metadata, metadata_fname)) != 0)
    {
        spdlog::get("pm_logger")->error("Parse error: ParseManager::RecordMetadata failure");
        ch10_input_stream.close();
        return retcode;
    }

    ch10_input_stream.close();
    return EX_OK;
}

int ParseManager::Configure(const ManagedPath* input_ch10_file_path, ManagedPath output_dir,
    const ParserConfigParams& user_config, ParseManagerFunctions* pmf, 
	ParserPaths* parser_paths, ParserMetadata* metadata, std::vector<WorkUnit>& work_units,
    std::ifstream& ch10_stream)
{
    bool success = false;
    uint64_t ch10_file_size = 0;
    input_ch10_file_path->GetFileSize(success, ch10_file_size);
    if (!success)
    {
        spdlog::get("pm_logger")->error("Configure error: GetFileSize failure");
        return EX_IOERR;
    }
    spdlog::get("pm_logger")->info("Ch10 file size: {:f} MB", ch10_file_size / (1000.0 * 1000.0));

    uint64_t chunk_bytes = 0;
    uint16_t worker_count = 0;
    pmf->IngestUserConfig(user_config, ch10_file_size, chunk_bytes, worker_count);

    if(!parser_paths->CreateOutputPaths(*input_ch10_file_path, output_dir, 
        user_config.ch10_packet_enabled_map_, worker_count))
    {
        spdlog::get("pm_logger")->error("Configure error: CreateOutputPaths failure");
        return EX_CANTCREAT;
    }

    if(!pmf->MakeWorkUnits(work_units, worker_count, chunk_bytes, append_chunk_size_bytes_, 
        ch10_file_size, ch10_stream, parser_paths))
    {
        spdlog::get("pm_logger")->error("Configure error: MakeWorkUnits failure");
        return EX_SOFTWARE;
    }

    int retcode = 0;
    if((retcode = metadata->Initialize(*input_ch10_file_path, user_config, *parser_paths)) != 0)
    {
        spdlog::get("pm_logger")->error("Configure error: ParserMetadata::Initialize failure");
        return retcode;
    }

    if(!pmf->OpenCh10File(*input_ch10_file_path, ch10_stream))
    {
        spdlog::get("pm_logger")->error("Configure error: OpenCh10File failure");
        return EX_IOERR;
    }

    return EX_OK;
}

int ParseCh10(std::vector<WorkUnit*>& work_units, ParseManagerFunctions* pmf,
	ParseManager* pm, const ParserConfigParams& user_config)
{
    bool append = false;
    int retcode = 0;
    uint16_t effective_worker_count = static_cast<uint16_t>(work_units.size());
    std::vector<uint16_t> active_workers_vec;

    spdlog::get("pm_logger")->debug("Parse: begin parsing with workers");
    if ((retcode = pm->StartThreads(append, active_workers_vec, effective_worker_count, user_config,
        work_units, pmf)) != 0)
    {
        spdlog::get("pm_logger")->warn("Parse: Returning after first WorkerQueue");
        return retcode;
    }
    spdlog::get("pm_logger")->debug("Parse: end parsing with workers");

    // Wait for all active workers to finish.
    if (!pm->StopThreads(work_units, active_workers_vec, user_config.worker_shift_wait_ms_,
        pmf))
        return EX_SOFTWARE;

    append = true;
    if (effective_worker_count > 1)
        effective_worker_count--;
    else
        return EX_OK;

    spdlog::get("pm_logger")->debug("Parse: begin parsing in append mode");
    if ((retcode = pm->StartThreads(append, active_workers_vec, effective_worker_count, user_config,
        work_units, pmf)) != 0)
    {
        spdlog::get("pm_logger")->warn("Parse: Returning after append mode WorkerQueue");
        return retcode;
    }
    spdlog::get("pm_logger")->debug("Parse: end parsing in append mode");

    // Wait for all active workers to finish.
    if (!pm->StopThreads(work_units, active_workers_vec, user_config.worker_shift_wait_ms_,
        pmf))
        return EX_SOFTWARE;
    spdlog::get("pm_logger")->info("Parse: Parsing complete with no errors");

    return EX_OK;
}

int ParseManager::StartThreads(bool append_mode, 
                               std::vector<uint16_t>& active_workers_vec,
                               const uint16_t& effective_worker_count,
                               const ParserConfigParams& user_config, 
                               std::vector<WorkUnit*>& work_units, ParseManagerFunctions* pmf)
{
    std::chrono::milliseconds worker_offset_wait_ms(user_config.worker_offset_wait_ms_);
    std::chrono::milliseconds worker_shift_wait_ms(user_config.worker_shift_wait_ms_);

    uint16_t active_thread_count = 0;
    bool thread_started = false;
    uint64_t total_read_pos = 0;

    uint16_t worker_ind = 0;
    if(append_mode)
        worker_ind++;

    spdlog::get("pm_logger")->debug("StartThreads: Starting worker threads");
    for (worker_ind; worker_ind < effective_worker_count; worker_ind++)
    {
        thread_started = false;

        while (!thread_started)
        {
            if((retcode_ = ActivateInitialThread(pmf, append_mode, work_units.at(worker_ind),
                worker_offset_wait_ms, active_workers_vec, worker_ind, total_read_pos, 
                active_thread_count, user_config.parse_thread_count_, thread_started)) != 0)
                return retcode_;

            // The first worker reads TMATs only. In this case the next
            // worker should start immediately where the first worker
            // ends. Note that worker_index should never be 0 for append
            // mode since the first worker stops on a packet boundary
            // immediately after reading TMATs. Thus append mode is
            // only valid for the second worker and the following workers.
            if(worker_ind == 0)
            {
                spdlog::get("pm_logger")->debug("ParseManager::ActivateInitialThread(): "
                    "First worker: Waiting for first worker to stop "
                    "to collect TMATs data");
                if (!StopThreads(work_units, active_workers_vec, user_config.worker_shift_wait_ms_, pmf))
                    return EX_SOFTWARE;
                active_thread_count = 0;
                total_read_pos = work_units.at(worker_ind)->conf_.last_position_;
                // if(!CopyTMATSDataToWorkers(work_units)) 
            }
            
            if((retcode_ = ActivateAvailableThread(pmf, append_mode, work_units, worker_shift_wait_ms,
                active_workers_vec, worker_ind, total_read_pos, thread_started)) != 0)
                return retcode_;
        }  
    }  
    return EX_OK;
}

int ParseManager::ActivateInitialThread(ParseManagerFunctions* pmf, bool append_mode,
    WorkUnit* work_unit, std::chrono::milliseconds worker_wait_ms,
    std::vector<uint16_t>& active_workers,
    const uint16_t& worker_index, uint64_t& read_pos, 
    uint16_t& active_thread_count, const uint16_t& conf_thread_count, bool& thread_started)
{
    thread_started = false;
    if(active_thread_count < conf_thread_count)
    { 
        spdlog::get("pm_logger")->debug("ActivateInitialThread: All threads NOT ACTIVE ({:d} active)", 
            active_thread_count);

        if(!pmf->ActivateWorker(work_unit, active_workers, worker_index, append_mode, read_pos))
            return EX_SOFTWARE;

        active_thread_count += 1;
        read_pos += work_unit->GetReadBytes();
        thread_started = true;

        if (!append_mode)
            std::this_thread::sleep_for(worker_wait_ms);
    }
    return EX_OK;
}

int ParseManager::ActivateAvailableThread(ParseManagerFunctions* pmf, bool append_mode,
    std::vector<WorkUnit*>& work_units, std::chrono::milliseconds worker_wait_ms, 
    std::vector<uint16_t>& active_workers,
    const uint16_t& worker_index, uint64_t& read_pos, bool& thread_started)
{
    if(!thread_started)
    {
        spdlog::get("pm_logger")->debug("All threads ACTIVE");

        // Check active workers to see if they are ready to be joined.
        uint16_t current_active_worker = 0;
        for (uint16_t active_worker_ind = 0; active_worker_ind < active_workers.size();
                active_worker_ind++)
        {
            current_active_worker = active_workers[active_worker_ind];

            // Join workers that are complete and start the worker associated
            // with the current worker_ind
            if (work_units.at(current_active_worker)->IsComplete())
            {
                spdlog::get("pm_logger")->debug(
                    "ActivateAvailableThread: worker {:d} "
                    "INACTIVE/COMPLETE -- joining now",
                    current_active_worker);

                int retcode = work_units.at(current_active_worker)->ReturnValue();

                // Join the recently completed worker
                pmf->JoinWorker(work_units.at(current_active_worker), active_workers,
                    active_worker_ind);

                if(retcode != 0)
                {
                    spdlog::get("pm_logger")->error("ActivateAvailableThread: worker {:d} "
                        "retcode = {:d}", current_active_worker, retcode);
                    return retcode;
                }

                if(!pmf->ActivateWorker(work_units.at(worker_index), active_workers, 
                    worker_index, append_mode, read_pos))
                    return EX_SOFTWARE;

                thread_started = true;
                read_pos += work_units.at(worker_index)->GetReadBytes();
                break;
            }
            else
            {
                spdlog::get("pm_logger")->debug("ActivateAvailableThread: worker {:d} STILL ACTIVE", 
                    current_active_worker);
            }
        }

        // Wait before checking for available workers.
        spdlog::get("pm_logger")->trace("ActivateAvailableThread: waiting for workers");
        std::this_thread::sleep_for(worker_wait_ms);
    }
    return EX_OK;
}

bool ParseManager::StopThreads(std::vector<WorkUnit*>& work_units,
                                     std::vector<uint16_t>& active_workers_vec,
                                     int worker_shift_wait, ParseManagerFunctions* pmf)
{
    std::chrono::milliseconds worker_shift_wait_ms(worker_shift_wait);
    spdlog::get("pm_logger")->debug("StopThreads: Joining all remaining workers");
    uint16_t current_active_worker = 0;

    while (active_workers_vec.size() > 0)
    {
        for (uint16_t active_worker_ind = 0; active_worker_ind < active_workers_vec.size();
             active_worker_ind++)
        {
            // Join workers that are complete
            current_active_worker = active_workers_vec[active_worker_ind];
            if (work_units.at(current_active_worker)->IsComplete() == true)
            {
                spdlog::get("pm_logger")->debug(
                    "StopThreads: worker {:d} INACTIVE/COMPLETE -- joining now",
                    current_active_worker);

                pmf->JoinWorker(work_units.at(current_active_worker), active_workers_vec,
                    active_worker_ind);

                spdlog::get("pm_logger")->debug("StopThreads: worker {:d} joined", 
                    current_active_worker);
            }
            else
            {
                spdlog::get("pm_logger")->debug("StopThreads: worker {:d} STILL ACTIVE", 
                    current_active_worker);
            }
        }

        // Wait before checking for available workers.
        spdlog::get("pm_logger")->debug("StopThreads: waiting for workers to complete");
        std::this_thread::sleep_for(worker_shift_wait_ms);
    }

    spdlog::get("pm_logger")->debug("StopThreads: all workers joined");
    return true;
}

bool ParseManager::RecordMetadata(std::vector<WorkUnit*>& work_units, 
    ParserMetadata* metadata, const ManagedPath& metadata_fname)
{
    std::vector<const Ch10Context*> ctx_vec;
    for(std::vector<WorkUnit*>::iterator it = work_units.begin(); it != work_units.end(); ++it)
        ctx_vec.push_back((*it)->ctx_.get());

    if(!metadata->RecordMetadata(metadata_fname, ctx_vec))
        return false;

    return true;
}

void ParseManagerFunctions::IngestUserConfig(const ParserConfigParams& user_config,
    	const uint64_t& ch10_file_size, uint64_t& chunk_bytes,
		uint16_t& worker_count)
{
    // Multiply by 1e6 because this configuration parameters is in units
    // of MB.
    chunk_bytes = user_config.parse_chunk_bytes_ * 1e6;

    // Calculate the number of workers necessary to parse the entire file
    // based on the chunk of binary that each worker will parse.
    // Add one additional worker for the initial worker which only parses
    // TMATS packets then returns. 
    worker_count = static_cast<int>(ceil(static_cast<double>(ch10_file_size) 
        / static_cast<double>(chunk_bytes))) + 1;

    spdlog::get("pm_logger")->info("AllocateResources: chunk size {:d} bytes", 
        chunk_bytes);
    spdlog::get("pm_logger")->info("AllocateResources: using {:d} threads", 
        user_config.parse_thread_count_);

    // If the user-specified max_chunk_read_count is less the calculated worker_count_,
    // decrease the worker_count_ to max_chunk_read_count.
    if (user_config.max_chunk_read_count_ < worker_count)
        worker_count = user_config.max_chunk_read_count_;
    spdlog::get("pm_logger")->info("AllocateResources: creating {:d} workers", worker_count);
}

bool ParseManagerFunctions::MakeWorkUnits(std::vector<WorkUnit>& work_units, const uint16_t& worker_count,
    const uint64_t& read_size, const uint32_t& append_read_size,
    const uint64_t& total_size, std::ifstream& ch10_input_stream, const ParserPaths* parser_paths)
{
    work_units.resize(worker_count);
    for(uint16_t i = 0; i < worker_count; i++)
    {
        if(!work_units.at(i).conf_.Initialize(worker_count, i,
            read_size, append_read_size, total_size, ch10_input_stream, parser_paths))
            return false;
    }
    return true;
}

bool ParseManagerFunctions::ActivateWorker(WorkUnit* work_unit, 
    std::vector<uint16_t>& active_workers, const uint16_t& worker_index, 
    bool append_mode, const uint64_t& read_pos)
{
    if(!work_unit->CheckConfiguration(append_mode, read_pos))
        return false;

    work_unit->Activate();

    // Put worker in active workers list.
    active_workers.push_back(worker_index);
    return true;
}

bool ParseManagerFunctions::JoinWorker(WorkUnit* work_unit, std::vector<uint16_t>& active_workers,
			const uint16_t& active_worker_index)
{
    work_unit->Join();

    // Update the vector of active worker indices
    if(active_worker_index > (active_workers.size() - 1))
    {
        spdlog::get("pm_logger")->error("Error: active_worker_index = {:d} greater than array size",
            active_worker_index);
        return false;
    }
    active_workers.erase(active_workers.begin() + active_worker_index);

    return true;
}

bool ParseManagerFunctions::OpenCh10File(const ManagedPath& input_path, std::ifstream& input_stream)
{
    spdlog::get("pm_logger")->debug("Opening ch10 file path: {:s}", input_path.string());
    input_stream.open(input_path.string().c_str(), std::ios::binary);
    if (!(input_stream.is_open()))
    {
        spdlog::get("pm_logger")->error("Error opening file: {:s}", input_path.RawString());
        input_stream.close();
        return false;
    }

    return true;
}


bool ParseManagerFunctions::CopyTMATSDataToWorkers(std::vector<WorkUnit*>& work_units)
{
    // Check if first worker has TMATs matter.
    std::string tmats_raw = work_units.at(0)->ctx_->GetTMATSMatter();
    if(tmats_raw == "")
    {
        spdlog::get("pm_logger")->error("CopyTMATSDataToWorkers: "
        "First worker TMATs matter is an empty string");
        return false;
    }

    // Parse PCM attributes from TMATs data
    std::set<Ch10PacketType> pkts{Ch10PacketType::PCM_F1};
    TMATSData tmats_data;
    if(!tmats_data.Parse(tmats_raw, pkts))
    {
        spdlog::get("pm_logger")->error("CopyTMATSDataToWorkers: "
        "Failed to parse TMATs attributes");
        return false;
    }

    // Check presence of required attributes, cast to appropriate type,
    // and configure Ch10PCMTMATSData object
    Ch10PacketTypeSpecificMetadataFunctions mdf;
    unilateral_map index_to_code_and_vals = tmats_data.pcm_index_to_code_and_values;
    pcmdata_map pcmdata;
    for (unilateral_map::const_iterator it = index_to_code_and_vals.cbegin();
        it != index_to_code_and_vals.cend(); ++it)
    {
        Ch10PCMTMATSData temppcmdata;
        spdlog::get("pm_logger")->debug("CopyTMATSDataToWorkers: "
        "PopulatePCMDataObject for PCM TMATs index {:d}", it->first);
        if(!mdf.PopulatePCMDataObject(it->second, temppcmdata))
        {
            spdlog::get("pm_logger")->error("CopyTMATSDataToWorkers: "
            "PopulatePCMDataObject failed for PCM TMATs index {:d} due "
            "to missing required "
            "attributes or casting error", it->first);
            return false;
        }
        pcmdata[it->first] = temppcmdata;
    }

    for(std::vector<WorkUnit*>::const_iterator it = work_units.cbegin();
        it != work_units.cend(); it++)
    {
        (*it)->ctx_->SetPCMTMATSData(pcmdata);
    } 
    return true;
}