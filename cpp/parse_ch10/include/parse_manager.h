// parse_manager.h

/* ParseManager is called by the main() ch10parse entry point.
   It has functions and data members for managing multiple threads
   which in turn are each represented by a ParseWorker instance.
*/

#ifndef PARSEMANAGER_H
#define PARSEMANAGER_H

#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <memory>
#include <map>
#include <set>

#include "sysexits.h"
#include "parse_worker.h"
#include "parser_config_params.h"
#include "parser_paths.h"
#include "managed_path.h"
#include "worker_config.h"
#include "spdlog/spdlog.h"
#include "ch10_packet_type.h"
#include "ch10_pcm_tmats_data.h"
#include "ch10_context.h"
#include "parser_metadata.h"


class WorkUnit
{
	public:
		std::unique_ptr<Ch10Context> ctx_;
		std::unique_ptr<ParseWorker> worker_;
		std::thread thread_;
		WorkerConfig conf_;

		WorkUnit() : ctx_(std::make_unique<Ch10Context>()), worker_(std::make_unique<ParseWorker>()),
			thread_(), conf_()
		{}

		virtual bool CheckConfiguration(bool append_mode, const uint64_t& read_pos, 
			WorkerConfig* conf) 
		{ 
			conf->append_mode_ = append_mode;
			conf->start_position_ = read_pos;	
			return conf->CheckConfiguration(); 
		}

		virtual bool CheckConfiguration(bool append_mode, const uint64_t& read_pos) 
		{ 
			return CheckConfiguration(append_mode, read_pos, &(this->conf_)); 
		}

		virtual void Activate()
		{ thread_ = std::thread(std::ref(*worker_), std::ref(conf_), ctx_.get()); }

		virtual void Join()
		{ thread_.join(); }

		virtual const uint64_t& GetReadBytes() const { return conf_.actual_read_bytes_; }

		virtual bool IsComplete() const { return worker_->CompletionStatus(); }
		virtual int ReturnValue() const { return worker_->ReturnValue(); }
};

class ParseManagerFunctions
{
	public:
		ParseManagerFunctions()
		{}

		/*
		Calculate parameters from user configuration.

		Args:
			user_config			--> ParserConfigParams object which has been
									pre-configured with data from the
									parser_conf.yaml file
			ch10_file_size		--> Total size of the ch10 in bytes
			chunk_bytes			--> Initial byte count for each worker to ingest,
									as output var
			worker_count		--> Count of workers required to parse a file
									of size ch10_file_size, as output var

		Return:
			True if no errors, false if errors occur and
			execution ought to stop.
		*/
		virtual void IngestUserConfig(const ParserConfigParams& user_config,
			const uint64_t& ch10_file_size, uint64_t& chunk_bytes,
			uint16_t& worker_count);

		

		/*
		Open the ch10 file stream.

		Args:
			input_path				--> Ch10 file to parse
			input_stream			--> std::ifstream to open

		Return:
			True if no errors occur; false otherwise.
		*/
		virtual bool OpenCh10File(const ManagedPath& input_path, std::ifstream& input_stream);



		/*
		Activate a configured worker in a thread with the information contained
		within a single WorkUnit.

		Args:
			work_unit		--> A WorkUnit instance
			active_workers	--> Vector of active worker indices
			worker_index	--> Index of worker to be activated
			append_mode		--> True if append_mode is active
			read_pos		--> Read position of bytes in the total
								file

		Return:
			True if WorkUnit::CheckConfiguration returns true; false otherwise.
		*/
		virtual bool ActivateWorker(WorkUnit* work_unit, std::vector<uint16_t>& active_workers,
			const uint16_t& worker_index, bool append_mode, const uint64_t& read_pos);



		/*
		Join a worker relevant to a specific WorkUnit.

		Args:
			work_unit			--> A WorkUnit instance
			active_workers		--> Vector of active worker indices
			active_worker_index	--> Index of element in active_workers to be removed

		Return:
			True if no errors occur; false otherwise.
		*/
		virtual bool JoinWorker(WorkUnit* work_unit, std::vector<uint16_t>& active_workers,
			const uint16_t& active_worker_index);	
		


		/*
		Create a vector of WorkUnit in preparation for parsing.

		Args:
			work_units			--> Vector of WorkUnit to be populated
			worker_count		--> Count of WorkUnits to be populated
			read_size			--> Count of bytes to read for each worker, 
									aka chunk size
			append_read_size	--> Count of bytes to read in append mode	
			total_size			--> Count of bytes in total ch10 file
			ch10_input_stream	--> Stream from which to read bytes
			parsers_paths		--> Populate/configured ParserPaths instance

		Return:
			True if no errors occur; false otherwise.
		*/
		virtual bool MakeWorkUnits(std::vector<WorkUnit>& work_units, const uint16_t& worker_count,
			const uint64_t& read_size, const uint32_t& append_read_size,
			const uint64_t& total_size, std::ifstream& ch10_input_stream, const ParserPaths* parser_paths);

		/*
		Parse TMATs matter after first worker has completed to specifically
		extract data that is relevant to PCM packets. Cast parsed values
		and populate an object of Ch10PCMTMATSData then copy the object
		to each instance of Ch10Context so all subsequent workers have access
		to the PCM metadata. It is required that first worker (work_units[0])
		has already parsed and stored TMATs matter for this function to
		be meaningful. 

		Args:
		work_units			--> Vector of WorkUnit, each of which owns
								a Ch10Context to which PCM data will
								be copied.

		Return:
			False if a TMATs parsing error occurs, a TMATs metadata
			attribute can't be casted, or a required TMATs attribute
			isn't present, otherwise true. All parsing should stop if
			this function fails AND PCM packets are present. 

			Note: Check the last statement. This may not be universally
			true. 	 
		*/
		bool CopyTMATSDataToWorkers(std::vector<WorkUnit*>& work_units);
};

class ParseManager
{
	private:
		int retcode_; 

   public:
	// Count of bytes of raw ch10 data to be parsed by
    // each worker in append mode
	static const uint32_t append_chunk_size_bytes_;

	static const std::string metadata_filename_;

    ParseManager();
    virtual ~ParseManager();
	static uint32_t GetAppendChunkSizeBytes();
	static std::string GetMetadataFilename(); 


    //////////////////////////////////////////////////////////////////////////////
    // Functions below are considered to be internal functions. They
    // are made public to facilitate testing.
    //////////////////////////////////////////////////////////////////////////////


    /*
	Collect misc setup functions and validate configuration.

	Args:
		input_ch10_file_path	--> Ch10 file to parse
		output_dir				--> Directory in which to place
									parsed data
		config					--> ParserConfigParams object which has
									been pre-configured with data from
									the parser_conf.yaml file
		pmf						--> ParseManagerFunctions
		parser_paths			--> ParserPaths
		metadata				--> ParserMetadata
		work_units 				--> Vector of WorkUnit to be checked for completion,
									joined, and started
		ch10_stream				--> std::ifstream to be initialized

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
	virtual int Configure(const ManagedPath* input_ch10_file_path, ManagedPath output_dir,
        const ParserConfigParams& user_config, ParseManagerFunctions* pmf, 
		ParserPaths* parser_paths, ParserMetadata* metadata, std::vector<WorkUnit>& work_units,
		std::ifstream& ch10_stream);



	/*
	Execute all threads up to configured maximum thread count, then
	wait for threads to finish before starting the next thread.
	Continue this process until all WorkUnits have been processed.	

	Args:
		append_mode			--> True if current mode is append
		active_workers		--> Vector of active worker indices
		effective_worker_count> Count of WorkUnits to process
		config				--> ParserConfigParams object which has
								been pre-configured with data from
								the parser_conf.yaml file
		work_units			--> Vector of WorkUnit to be activated
		pmf					--> Instance of ParseManagerFunctions

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
	virtual int StartThreads(bool append_mode, 
                               std::vector<uint16_t>& active_workers_vec,
                               const uint16_t& effective_worker_count,
                               const ParserConfigParams& user_config, 
							   std::vector<WorkUnit*>& work_units,
							   ParseManagerFunctions* pmf);



	/*
	Wait for each worker to finish before returning. As each worker completes
	the parsing job asynchronously, join the thread and deallocate the buffer.

	Args:
		work_units				--> Vector of WorkUnit to be activated
		active_workers_vec		--> Track the index of active workers
		worker_shift_wait		--> Main thread sleep duration in loop after
									checking for any completed workers, millisecond
		pmf						--> Instance of ParseManagerFunctions

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    virtual bool StopThreads(std::vector<WorkUnit*>& work_units,
                           std::vector<uint16_t>& active_workers_vec,
                           int worker_shift_wait, ParseManagerFunctions* pmf);



	/*
	Activate a thread without waiting for a thread to complete first.
	Executes automatically if the active_thread_count is less than the
	config_thread_count.

	Args:
		pmf					--> Instance of ParseManagerFunctions
		append_mode			--> True if current mode is append
		work_unit			--> Instance of WorkUnit to be activated
		worker_wait_ms		--> Time to wait after starting thread
		active_workers		--> Vector of active worker indices
		worker_index		--> Index of worker to be activated
		read_pos			--> Read position of bytes in the total file
		active_thread_count	--> Count of currently active threads
		conf_thread_count	--> Configured maximum thread count
		thread_started		--> Output variable set true if a thread

	Return:
		True if no errors occurred. 
	*/
	int ActivateInitialThread(ParseManagerFunctions* pmf, bool append_mode,
		WorkUnit* work_unit, std::chrono::milliseconds worker_wait_ms, 
		std::vector<uint16_t>& active_workers,
		const uint16_t& worker_index, uint64_t& read_pos, 
		uint16_t& active_thread_count, const uint16_t& conf_thread_count, bool& thread_started);



	/*
	Activate a thread if one of the previously active threads has completed.
	First join the completed thread. Only executes if input thread_started
	is false.

	Args:
		pmf					--> Instance of ParseManagerFunctions
		append_mode			--> True if current mode is append
		work_units 			--> Vector of WorkUnit to be checked for completion,
								joined, and started
		worker_wait_ms		--> Time to wait after starting thread
		active_workers		--> Vector of active worker indices
		worker_index		--> Index of worker to be activated
		read_pos			--> Read position of bytes in the total file
		thread_started		--> Output variable set true if a thread

	Return:
		True if no errors occurred. 
	*/
	int ActivateAvailableThread(ParseManagerFunctions* pmf, bool append_mode,
		std::vector<WorkUnit*>& work_units, std::chrono::milliseconds worker_wait_ms, 
		std::vector<uint16_t>& active_workers,
		const uint16_t& worker_index, uint64_t& read_pos, bool& thread_started);



	/*
	Finalize the parse process by recording metadata.

	Args:
		work_units 				--> Vector of WorkUnit to be checked for completion,
									joined, and started
		metadata				--> Configured ParserMetadata instance
		metadata_fname			--> Metadata output filename	

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
	virtual bool RecordMetadata(std::vector<WorkUnit*>& work_units, 
		ParserMetadata* metadata, const ManagedPath& metadata_fname);

	
};

/*
High-level function which integrates all of the parsing activity.
Exists outside a class for testing purposes.

Args:
	work_units 			--> Vector of WorkUnit 
	pmf					--> ParseManagerFunctions instance
	pm					--> ParseManager instance
	user_config			--> ParserConfigParams object which has
							been pre-configured with data from
							the parser_conf.yaml file

Return:
	False if parsing fails in some way.
*/
int ParseCh10(std::vector<WorkUnit*>& work_units, ParseManagerFunctions* pmf,
	ParseManager* pm, const ParserConfigParams& user_config);



/*
Highest-level Function that accomplishes parsing then 
metadata recording.

Args:
	ch10_path		--> Ch10 input path
	out_dir			--> Output directory
	config			--> Populated instance of ParserConfigParams

Return:
	True if no errors occur.
*/
int Parse(ManagedPath ch10_path, ManagedPath out_dir, const ParserConfigParams& config);


#endif