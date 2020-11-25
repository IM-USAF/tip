#!/usr/bin/env bash
# CMAKE_BUILD_DIR defined by pipeline
# UNITTEST_REPORT_DIR defined by pipeline


# exit when any command fails
set -e


# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# printf an error message before exiting
trap 'printf "\"${last_command}\" command failed with exit code $?.\n"' ERR

# A command line argument is either a test directory or a log file to use in place of end-to-end testing
if [ -f "$1" ]; then
	LOG_FILE=$1
elif [ -d "$1" ]; then
	E2E_TEST=$1
elif [ -z "$1" ]; then
	if [ -d /test ]; then
		E2E_TEST=/test
	else
		printf "No file or directory specified and there is no /test directory\n"
		exit 1
	fi
else
	printf "\n'$1' is not a file or directory\n"
	printf "Usage: unit_test.sh [log_file | test_directory]\n"
	exit 1
fi

declare -A MAX_TIMES
MAX_TIMES=([parse]=0.0 [translate]=0.0)


BASE_DIR=${PWD}
if [ -z "${CMAKE_BUILD_DIR}" ]; then 
	LOCAL=True
	# We are not in the pipeline; set vars for running locally
	if [ -d /app ]; then 
		BASE_DIR=/app
	fi
	CMAKE_BUILD_DIR=${BASE_DIR}/build
	UNITTEST_REPORT_DIR=$BASE_DIR/reports
fi
TEST_DIR=${CMAKE_BUILD_DIR}/cpp

# Skip unit tests if a log file was specified for testing
if [ -z "$LOG_FILE" ]; then
	printf "\n-------------------- Unit Tests --------------------\n"

	# For now, run cpp/tests because it is much faster than ctest
	# In the future we might have to run ctest in order to get coverage statistics
	# If we do, try to make our tests compatible with the --parallel option of ctest
	cd ${TEST_DIR}
	./tests
	cd ${BASE_DIR}

	printf "\n-------------------- Test Coverage --------------------\n"

	if [ -x gcovr ]; then
		mkdir -p ${UNITTEST_REPORT_DIR}
		printf "Writing coverage reports in ${UNITTEST_REPORT_DIR}\n"
		GCOV=gcov
		GCOV="${GCOV}" gcovr -j --verbose \
			--exclude-unreachable-branches \
			--exclude-throw-branches \
			--object-directory="${TEST_DIR}" \
			--xml ${UNITTEST_REPORT_DIR}/overall-coverage.xml \
			--html ${UNITTEST_REPORT_DIR}/overall-coverage.html \
			--sonarqube ${UNITTEST_REPORT_DIR}/overall-coverage-sonar.xml \
			--filter "${CPP_COVERAGE_FILTER}" \
			$([ -n "${CPP_COVERAGE_EXCLUDE}" ] && printf --exclude="${CPP_COVERAGE_EXCLUDE}\n")
	else 
		printf "*** No gcovr found. Skipping test coverage ***\n"
		if [ -z "$LOCAL" ]; then
			exit 1
		fi
	fi

	printf "\n-------------------- End-to-End Test ---------------------\n"
	cd $BASE_DIR
	# In the pipeline, only the build directory is present.
	# Restore the bin directory if needed.
	if [ ! -d ./bin ]; then mv build/bin . ; fi

	# If in a test container
	if [ -z "E2E_TEST" ]; then
		printf "No test directory specified and directory /test does not exist"
		EXIT $1
	fi

	# Run end-to-end validator
	python tip_scripts/e2e_validation/run_end_to_end_validator.py --video $E2E_TEST/truth $E2E_TEST/test $E2E_TEST/log

	# get newest log file
	LOG_FILE="$(ls -1t $E2E_TEST/log/* | head -1)"

else
	printf "\n*** Log file specified on command line.  Skipping tests. ***\n"
fi


printf "\n-------------------- End-to-End Results ------------------\n"

if [[ -z "$LOG_FILE" ]]; then
	printf "FAIL: No log file was found.\n"
	exit 1
fi

EXIT_CODE=0 # Start by assuming success
if ! grep "Total raw 1553 data: PASS" "$LOG_FILE" ; then
	printf "ERROR: Parser validation failed\n"
	EXIT_CODE=1
fi

if ! grep "Total translated 1553 data: PASS" "$LOG_FILE" ; then
	printf "ERROR: Translator validation failed\n"
	# Continue without failing; the pipeline doesn't expect translation to succeed at this point
fi

function check_time {
	file=$1; shift
	key_word=$1; shift
	threshold=$1; shift
	return_value=0 # Assume success
	# Use the first 5 characters of the type in the pattern (e.g. Parse, Transl)
	pattern=$(printf "${key_word:0:5}.*:.*seconds")
	# printf "\nFile: $file \nPattern: $pattern \nThreshold: $threshold\n"

	# Get the line containing the total time
	# and save an array of the words
	words=( $(grep -ie "$pattern" $file) )
	# printf "\n${#words[*]} words \n${words[*]} \n"

	# Get the word before "seconds"
	if [ ${#words[*]} -gt 1 ]; then 
		value="${words[-2]}"
	fi
	# printf "Value: $value\n"

	if [ "$value" = "None" ]; then
		return_value=1
		printf "None\n"
	else
		# Use python to compare floating point values
		if [ $(python -c "print($value < $threshold)") != "True" ]; then
			return_value=1
			printf "FAIL: $value is not less than $threshold seconds\n"
		else
			printf "PASS\n"
		fi
	fi

	return $return_value
}

# Check times
for type in ${!MAX_TIMES[@]} ; do
	printf "\n$type time: "
	if ! check_time $LOG_FILE $type ${MAX_TIMES[$type]} ; then
		# For now, do not fail for translate time
		if [ "$type" != "Translate" ]; then
			EXIT_CODE=1
		fi
	fi
	
done

exit $EXIT_CODE