#include "parser_helper_funcs.h"

bool ValidateConfig(ParserConfigParams& config, std::string config_path,
	ManagedPath& final_config_path)
{
	ManagedPath conf_path;
	if (config_path == "")
		conf_path = conf_path.parent_path() / "conf" / "parse_conf.yaml";
	else
		conf_path = ManagedPath(config_path) / "parse_conf.yaml";
	/*printf("Configuration file path: %s\n", conf_path.RawString().c_str());*/
	bool settings_validated = config.Initialize(conf_path.string());
	final_config_path = conf_path;
	return settings_validated;
}

bool ValidatePaths(char* arg1, char* arg2, ManagedPath& input_path, ManagedPath& output_path)
{
	// Get path to ch10 file. 
	std::string arg_path = arg1;
	input_path = ManagedPath(arg_path);
	if (!input_path.is_regular_file())
	{
		printf("User-defined input path is not a file/does not exist: %s\n",
			input_path.RawString().c_str());
		return false;
	}
	printf("Ch10 file path: %s\n", input_path.RawString().c_str());

	// Check for a second argument. If present, this path specifies the output
	// path. If not present, the output path is the same as the input path.
	output_path = input_path.parent_path();
	if ((arg2 != NULL) && (strlen(arg2) != 0))
	{
		output_path = ManagedPath(std::string(arg2));
		if (!output_path.is_directory())
		{
			printf("User-defined output path is not a directory: %s\n",
				output_path.RawString().c_str());
			return false;
		}
	}
	printf("Output path: %s\n", output_path.RawString().c_str());
	return true;
}

bool StartParse(ManagedPath input_path, ManagedPath output_path,
	ParserConfigParams config, double& duration)
{
	// Get start time.
	auto start_time = std::chrono::high_resolution_clock::now();

	// Initialization includes parsing of TMATS data.
	ParseManager pm(input_path, output_path, &config);

	if (pm.error_state())
		return false;

	// Begin parsing of Ch10 data by starting workers.
	pm.start_workers();

	// Get stop time and print duration.
	auto stop_time = std::chrono::high_resolution_clock::now();
	duration = (stop_time - start_time).count() / 1.0e9;
	printf("Duration: %.3f sec\n", duration);
	return true;
}

bool SetupLogging()
{
	try
	{
		// Setup async thread pool.
		//spdlog::init_thread_pool(8192, 2);

		// Rotating logs maxima
		int max_size = 1024 * 1024 * 10; // 10 MB
		int max_files = 20; 

		// Console sink
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::info);
		console_sink->set_pattern("[%T] [%n] [%l] %v");

		// ParseManager log
		// automatically registered?
		auto pm_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>("logs/pm_log.txt",
			max_size, max_files);
		pm_log_sink->set_level(spdlog::level::trace);
		pm_log_sink->set_pattern("[%D %T] [%l] %v");

		// List of sinks for ParseManager, console logger
		spdlog::sinks_init_list pm_sinks = { console_sink, pm_log_sink };

		// Create and register the logger for ParseManager log and console.
		auto pm_logger = std::make_shared<spdlog::logger>("pm", pm_sinks.begin(), pm_sinks.end());
		spdlog::register_logger(pm_logger);

		/*auto pm_logger = spdlog::create_async<spdlog::sinks::rotating_file_sink>("pm_logger",
			"logs/pm_log.txt", max_size, max_files);*/


	}
	catch (const spdlog::spdlog_ex& ex)
	{
		printf("SetupLogging() failed: %s\n", ex.what());
		return false;
	}
	return true;
}