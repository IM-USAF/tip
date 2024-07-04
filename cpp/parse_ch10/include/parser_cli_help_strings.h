#ifndef PARSER_CLI_HELP_H_
#define PARSER_CLI_HELP_H_

#include <string>
#include <cstdint>

const std::string high_level_description = 
    R"(Parse Ch10 files and generate Parquet output)";

const std::string help_request_help = 
    R"(Print usage information.)";

const std::string version_request_help = 
    R"(Print version, either latest commit git SHA or tag.)";

const std::string input_path_help = 
    R"(Ch10 input file path)";

const std::string output_path_help = 
    R"(Output directory for artifacts created at parse time)";

const std::string log_dir_help = 
    R"(Log files output directory.)";

const std::string disable_1553f1_help = 
    R"(Disable parsing of MILSTD1553_FORMAT1 packets)";

const std::string disable_videof0_help = 
    R"(Disable parsing of VIDEO_FORMAT0 packets)";

const std::string disable_eth0_help = 
    R"(Disable parsing of ETHERNET_FORMAT0 packets)";

const std::string disable_arinc0_help = 
    R"(Disable parsing of ARINC429_FORMAT0 packets)";

const std::string disable_pcmf1_help = 
    R"(Disable parsing of PCM_F1 packets)";

const std::string chunk_bytes_help =
    R"(Quantity of bytes, in units of millions, ingested for parsing by each worker thread. Must be 
       in range [135, 1000].)";

const std::string thread_count_help =
    R"(Quantity of simultaneous threads. As a general rule of thumb, use 3/4 of your logical processors.
       Valid ranges are [1, Cores * 1.5]. Consider that at least parse_thread_count * parse_chunk_bytes 
       of memory will be consumed.)";

const std::string max_read_count_help = 
    R"(Maximum quantity of worker threads that will be started to ingest and parse the ch10 file.
       Total parse size = parse_chunk_bytes * max_chunk_read_count. If total parse size is less than
       ch10 file size, the ch10 file will not be fully parsed.)";

const std::string worker_offset_wait_ms_help = 
    R"(Duration in millisecond to wait after each worker starts, only for the initial set of 
       parse_thread_count workers. Staggered start time distributes IO load. Normally in 
       range [500, 10000].)";

const std::string worker_shift_wait_help = 
    R"(Duration in millisecond to wait before checking workers' states. Normally in range [20, 500].)";

const std::string stdout_log_level_help = 
    R"(Set minimum log level of stdout. All log entries
with level value greater or equal to minimum level value will
be printed to stdout.
**||
Log level and value:
- trace     = 0
- debug     = 1
- info      = 2
- warn      = 3
- error     = 4
- critical  = 5
- off       = 6

**||
Accept values in the set {trace, debug, info, warn, error, critical, off}.)";

const std::string file_log_level_help = 
    R"(Set minimum log level of log files. All log entries
with level value greater or equal to minimum level value will
be printed to stdout.
**||
Log level and value:
- trace     = 0
- debug     = 1
- info      = 2
- warn      = 3
- error     = 4
- critical  = 5
- off       = 6

**||
Accept values in the set {trace, debug, info, warn, error, critical, off}.)";


#endif  // PARSER_CLI_HELP_H_