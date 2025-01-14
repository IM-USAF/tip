set(headers
    include/logger_setup.h
    include/parquet_context_mock.h
    include/argument_validation_mock.h
    include/parse_manager_mock.h
    include/file_reader_mock.h
    include/dts1553_mock.h
    include/managed_path_mock.h
    include/tip_md_document_mock.h
    include/binbuff_mock.h
    include/ch10_context_mock.h
    include/ch10_packet_header_component_mock.h
    include/dts_429_mock.h
    include/organize_429_icd_mock.h
    include/arinc429_data_mock.h
    include/ch10_time_mock.h
    include/ch10_tmats_component_mock.h
    include/ch10_tdp_component_mock.h
    include/ch10_1553f1_component_mock.h
    include/ch10_videof0_component_mock.h
    include/ch10_ethernetf0_component_mock.h
    include/ch10_arinc429f0_component_mock.h
    include/parquet_tdpf1_mock.h
    include/tmats_data_mock.h
    include/parser_metadata_mock.h
    include/ch10_packet_type_specific_metadata_mock.h
    include/parser_paths_mock.h
    include/worker_config_mock.h
)

add_executable(tests
    main/test.cpp
    src/argument_validation_u.cpp
    src/binbuff_u.cpp
    src/bus_map_u.cpp
    src/ch10_1553f1_component_u.cpp
    src/ch10_context_u.cpp
    src/ch10_ethernetf0_component_u.cpp
    src/ch10_packet_component_u.cpp
    src/ch10_packet_element_u.cpp
    src/ch10_packet_header_component_u.cpp
    src/ch10_packet_u.cpp
    src/ch10_pcm_tmats_data_u.cpp
    src/ch10_pcmf1_component_u.cpp
    src/ch10_tdp_component_u.cpp
    src/ch10_time_u.cpp
    src/ch10_tmats_component_u.cpp
    src/ch10_videof0_component_u.cpp
    src/dts1553_u.cpp
    src/file_reader_u.cpp
    src/icd_element_u.cpp
    src/icd_data_u.cpp
    src/icd_translate_u.cpp
    src/iterable_tools_u.cpp
    src/logger_setup.cpp
    src/managed_path_u.cpp
    src/network_packet_parser_u.cpp
    src/parquet_arrow_validator_u.cpp
    src/parquet_context_u.cpp
    src/parquet_reader_u.cpp
    src/parse_manager_u.cpp
    src/parser_config_params_u.cpp
    src/parse_text_u.cpp
    src/parse_worker_u.cpp
    src/tmats_parser_u.cpp
    src/tmats_data_u.cpp
    src/translation_config_params_u.cpp
    src/user_input_u.cpp
    src/yaml_reader_u.cpp
    src/yaml_schema_validation_u.cpp
    src/yamlsv_log_item_u.cpp
    src/resource_limits_u.cpp
    src/version_info_u.cpp
    src/uri_percent_encoding_u.cpp
    src/translate_tabular_context_base_u.cpp
    src/translate_tabular_u.cpp
    src/translate_tabular_context_1553_u.cpp
    src/translatable_table_base_u.cpp
    src/translatable_column_base_u.cpp
    src/translatable_column_template_u.cpp
    src/translate_tabular_parquet_u.cpp
    src/stream_buffering_u.cpp
    src/sha256_tools_u.cpp
    src/ch10_arinc429f0_component_u.cpp
    src/md_category_u.cpp
    src/md_category_scalar_u.cpp
    src/md_category_sequence_u.cpp
    src/md_category_map_u.cpp
    src/md_document_u.cpp
    src/yaml_md_document_u.cpp
    src/tip_md_document_u.cpp
    src/provenance_data_u.cpp
    src/dts429_u.cpp
    src/parquet_arinc429f0_u.cpp
    src/organize_429_icd_u.cpp
    src/parquet_milstd1553f1_u.cpp
    src/parquet_videodataf0_u.cpp
    src/parquet_ethernetf0_u.cpp
    src/ch10_parse_main_u.cpp
    src/translate_tabular_1553_main_u.cpp
    src/arinc429_data_u.cpp
    src/translate_tabular_arinc429_main_u.cpp
    src/parquet_tdpf1_u.cpp
    src/cli_arg_u.cpp 
    src/cli_positional_arg_u.cpp 
    src/cli_optional_arg_u.cpp
    src/cli_flag_u.cpp
    src/cli_u.cpp
    src/cli_group_u.cpp
    src/container_arg_u.cpp
    src/parser_paths_u.cpp
    src/parser_metadata_u.cpp
    src/ch10_packet_type_specific_metadata_u.cpp
    src/worker_config_u.cpp
)

target_compile_features(tests PUBLIC cxx_std_17)

target_include_directories(tests
    PUBLIC include
)

target_link_libraries(tests
    PUBLIC common yaml-cpp tiputil spdlog::spdlog yamlsv cli metadata 
    sha256 ch10_parquet parquet_context ch10ethernet ch10comp
    dts_1553 dts_429 parquet_compare pq_vid_extract 
    translate_tabular translate_tabular_1553
    translate_tabular_arinc429 parsech10_lib 
    PRIVATE ${GTEST_LIBRARIES}
)

install(
    TARGETS tests
    DESTINATION ${out_bin_dir}
)

enable_testing()
if(NOT BUILD_SHARED_LIBS)
    include(GoogleTest)
    gtest_discover_tests(tests)
endif()
