#ifndef PARSER_CLI_H_
#define PARSER_CLI_H_

#include <memory>
#include <string>
#include <set>
#include "cli_group.h"
#include "parser_cli_help_strings.h"
#include "parser_config_params.h"

inline bool ConfigureParserCLI(CLIGroup& cli_group, ParserConfigParams& config, 
    bool& help_requested, bool& show_version)
{
    ////////////////////////////////////////////////////////////////////////////////
    //                                  help CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_help = cli_group.AddCLI(CH10_PARSE_EXE_NAME, 
        high_level_description, "clihelp");
    cli_help->AddOption("--help", "-h", help_request_help, false, 
        help_requested, true);


    ////////////////////////////////////////////////////////////////////////////////
    //                                  version CLI
    ////////////////////////////////////////////////////////////////////////////////

    // std::shared_ptr<CLIGroupMember> cli_version = cli_group.AddCLI(CH10_PARSE_EXE_NAME, 
    //     high_level_description, "cliversion");
    // cli_version->AddOption("--version", "-v", version_request_help, 
    //     false, show_version, true);


    ////////////////////////////////////////////////////////////////////////////////
    //                                  full CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli = cli_group.AddCLI(CH10_PARSE_EXE_NAME,
        high_level_description, "clifull");

    // Positional args, required
    cli->AddOption("input_path", input_path_help, config.input_path_str_, true);

    // Optional args
    cli->AddOption<std::string>("--output_path", "-o", output_path_help, "<input path directory>", 
        config.output_path_str_)->DefaultUseParentDirOf(config.input_path_str_);

    cli->AddOption<std::string>("--log_path", "-l", log_dir_help, "<output path directory>",
        config.log_path_str_)->DefaultUseValueOf(config.output_path_str_);

    cli->AddOption("--chunk_size", "-c", chunk_bytes_help, 200, 
        config.parse_chunk_bytes_)->ValidateInclusiveRangeIs(135, 1000);
    cli->AddOption("--thread_count", "-t", thread_count_help, 1, 
        config.parse_thread_count_)->ValidateInclusiveRangeIs(1, 256);
    cli->AddOption("--max_read_count", "-m", max_read_count_help, 1000, 
        config.max_chunk_read_count_)->ValidateInclusiveRangeIs(1, 10000);
    cli->AddOption("--worker_offset", "", worker_offset_wait_ms_help, 200, 
        config.worker_offset_wait_ms_)->ValidateInclusiveRangeIs(1, 30000);
    cli->AddOption("--worker_wait", "", worker_shift_wait_help, 200, 
        config.worker_shift_wait_ms_)->ValidateInclusiveRangeIs(1, 30000);

    std::set<std::string> permitted_log_levels{"trace", "debug", "info", "warn", "error", 
        "critical", "off"};
    cli->AddOption<std::string>("--stdout_log_level", "-L", stdout_log_level_help, "info", 
        config.stdout_log_level_)->ValidatePermittedValuesAre(permitted_log_levels);
    cli->AddOption<std::string>("--file_log_level", "-F", file_log_level_help, "info", 
        config.file_log_level_)->ValidatePermittedValuesAre(permitted_log_levels);

    // Flags
    cli->AddOption("--disable_1553f1", "", disable_1553f1_help, false, config.disable_1553f1_);
    cli->AddOption("--disable_videof0", "", disable_videof0_help, false, config.disable_videof0_);
    cli->AddOption("--disable_eth0", "", disable_eth0_help, false, config.disable_eth0_);
    cli->AddOption("--disable_arinc0", "", disable_arinc0_help, false, config.disable_arinc0_);
    cli->AddOption("--disable_pcmf1", "", disable_pcmf1_help, false, config.disable_pcmf1_);

    if(!cli_group.CheckConfiguration())
        return false;
    return true;
}

#endif  // PARSER_CLI_H_