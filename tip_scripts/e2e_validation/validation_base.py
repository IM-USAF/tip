import os
import re
from pathlib import Path

class ValidationBase(object):

    def __init__(self, prefix, debug=0):
        self.test_path = None
        self.truth_path = None
        self.test_passed = None
        self.regex_translated_1553_msg_dir = re.compile(".+_1553_translated_.+parquet")
        self.regex_raw_1553_dir = re.compile(".+_1553.parquet")
        self.regex_raw_video_dir = re.compile(".+_video.parquet")
        self.ready_to_validate = False
        self.prefix = prefix
        self.truth_dir_exists = None
        self.test_dir_exists = None
        self.truth_file_exists = None
        self.test_file_exists = None

    def get_test_result(self):
        return self.test_passed

    def get_test_result_string(self):
        return self.validation_result_string(self.test_passed)

    def get_test_result_string_from_input(self, input_result):
        return self.validation_result_string(input_result)

    def set_paths(self, truth_path, test_path):

        self.test_path = test_path
        self.truth_path = truth_path

        if self.truth_path is None:
            print('{:s} - {:s} is None!'.format(self.prefix, self.truth_path))
            return False
           
        if self.test_path is None:
            print('{:s} - {:s} is None!'.format(self.prefix, self.test_path))
            return False

        return True

    def validation_result_string(self, test_passed):
        if test_passed:
            return 'PASS'
        elif test_passed is None:
            return 'NULL'
        elif not test_passed:
            return 'FAIL'
        else:
            return 'BAD RESULT'

    def set_directory_paths(self, truth_path, test_path):

        if not self.set_paths(truth_path, test_path):
            return False

        if not os.path.isdir(self.truth_path):
           print('{:s} - {:s} is not a directory!'.format(self.prefix, self.truth_path))
           self.truth_dir_exists = False
           return False
        self.truth_dir_exists = True

        if not os.path.isdir(self.test_path):
            print('{:s} - {:s} is not a directory!'.format(self.prefix, self.test_path))
            self.test_dir_exists = False
            return False
        self.test_dir_exists = True

        return True

    def set_file_paths(self, truth_path, test_path):

        if not self.set_paths(truth_path, test_path):
            return False

        if not os.path.isfile(self.truth_path):
            print('{:s} - {:s} is not a file!'.format(self.prefix, self.truth_path))
            self.truth_file_exists = False
            return False
        self.truth_file_exists = True
           
        if not os.path.isfile(self.test_path):
            print('{:s} - {:s} is not a file!'.format(self.prefix, self.test_path))
            self.test_file_exists = False
            return False
        self.test_file_exists = True

        return True

    def set_1553_paths(self, truth_path, test_path, type_str):

        if not self.set_paths(truth_path, test_path):
            return False

        if type_str == 'transl1553':
            if not self._is_translated_1553_msg_dir(self.truth_path):
                print('{:s} - {:s} is not a translated 1553 msg dir!'.format(self.prefix, self.truth_path))
                self.truth_dir_exists = False
                return False
            self.truth_dir_exists = True
            if not self._is_translated_1553_msg_dir(self.test_path):
                print('{:s} - {:s} is not a translated 1553 msg dir!'.format(self.prefix, self.test_path))
                self.test_dir_exists = False
                return False
            self.test_dir_exists = True
        elif type_str == 'raw1553':
            if not self._is_raw_1553_dir(self.truth_path):
                print('{:s} - {:s} is not a raw 1553 dir!'.format(self.prefix, self.truth_path))
                self.truth_dir_exists = False
                return False
            self.truth_dir_exists = True
            if not self._is_raw_1553_dir(self.test_path):
                print('{:s} - {:s} is not a raw 1553 dir!'.format(self.prefix, self.test_path))
                self.test_dir_exists = False
                return False
            self.test_dir_exists = True
        elif type_str == 'rawvideo':
            if not self._is_raw_video_dir(self.truth_path):
                print('{:s} - {:s} is not a raw 1553 dir!'.format(self.prefix, self.truth_path))
                self.truth_dir_exists = False
                return False
            self.truth_dir_exists = True
            if not self._is_raw_video_dir(self.test_path):
                print('{:s} - {:s} is not a raw 1553 dir!'.format(self.prefix, self.test_path))
                self.test_dir_exists = False
                return False
            self.test_dir_exists = True
        else:
            print('ValidationBase.set_1553_paths(): type_str = {:s} not defined!'.format(type_str))
            return False

        return True

    def _is_translated_1553_msg_dir(self, input_path):
        if not os.path.isdir(input_path):
            return False

        if bool(re.match(self.regex_translated_1553_msg_dir, input_path)):
            return True
        return False

    def _is_raw_1553_dir(self, input_path):
        if not os.path.isdir(input_path):
            return False

        if bool(re.match(self.regex_raw_1553_dir, input_path)):
            return True
        return False

    def _is_raw_video_dir(self, input_path):
        if not os.path.isdir(input_path):
            return False

        if bool(re.match(self.regex_raw_video_dir, input_path)):
            return True
        return False

    def __repr__(self):
        r = '{:s}\ntruth: {:s}\ntest: {:s}'.format(self.prefix,
                                                   str(self.truth_path),
                                                   str(self.test_path))
        return r






