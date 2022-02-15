// ch10parse.h

#ifndef ARROW_STATIC
#error arrow_static must be defined in conjunction with parquet
#endif

#ifndef PARQUET_STATIC
#error parquet_static must be defined in conjunction with parquet
#endif

#ifndef CH10_PARSE_EXE_NAME
#error "ch10parse.cpp: CH10_PARSE_EXE_NAME must be defined"
#endif

#include "ch10_parse_main.h"

int main(int argc, char* argv[])  // GCOVR_EXCL_LINE
{
    return Ch10ParseMain(argc, argv);  // GCOVR_EXCL_LINE
}
