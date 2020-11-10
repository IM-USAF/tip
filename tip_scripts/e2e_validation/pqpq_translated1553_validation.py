from tip_scripts.e2e_validation import validation_base

class PqPqTranslated1553Validation(validation_base.ValidationBase):

    def __init__(self, truth_path, test_path, exec_path):
        prefix = 'PqPqTranslated1553Validation'
        validation_base.ValidationBase.__init__(self, prefix)
        self.ready_to_validate = self.set_1553_paths(truth_path, test_path, True)
        self.exec_path = exec_path

    def validate(self):
        return self.do_validation(self.exec_path)