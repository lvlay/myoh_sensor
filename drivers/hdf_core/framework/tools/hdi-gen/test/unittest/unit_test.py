#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
#
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.

import os
import subprocess
import time


def get_time_stamp():
    return int(round(time.time() * 1000))


def print_success(info):
    print("\033[32m{}\033[0m".format(info))


def print_failure(info):
    print("\033[31m{}\033[0m".format(info))


def is_subsequence(first_file, second_file):
    first_info = first_file.read()
    second_info = second_file.readline()
    while second_info:
        if first_info.find(second_info) == -1:
            print("line\n", second_info, "is not in output file")
            return False
        second_info = second_file.readline()
    return True


def compare_file(first_file_path, second_file_path):
    with open(first_file_path, 'r') as first_file:
        with open(second_file_path, 'r') as second_file:
            return is_subsequence(first_file, second_file)


def compare_target_files(first_file_path, second_file_path):
    first_files_list = get_all_files(first_file_path)
    second_files_list = get_all_files(second_file_path)

    first_files = set([file[len(first_file_path):] for file in first_files_list])
    second_files = set([file[len(second_file_path):-4] for file in second_files_list])

    common_files = first_files & second_files
    
    for files in common_files:
        if not compare_file("{}{}".format(first_file_path, files), "{}{}.txt".format(second_file_path, files)):
            print("file ", "{}{}".format(first_file_path, files), "{}{}.txt".format(second_file_path, files), \
                "is different")
            return False
    return True


def exec_command(command):
    return subprocess.getstatusoutput(command)


def file_exists(file_path):
    return os.path.isfile(file_path)


def make_binary_file(file_path):
    print("making hdi-gen...")
    return exec_command("make --directory={} --jobs=4".format(file_path))


def clean_binary_file(file_path):
    return exec_command("make --directory={} clean".format(file_path))


def get_all_files(path):
    file_list = []
    items = os.listdir(path)
    for item in items:
        item_path = os.path.join(path, item)
        if not os.path.isdir(item_path):
            file_list.append(item_path)
        else:
            file_list += get_all_files(item_path)
    return file_list


def get_all_idl_files(idl_path):
    file_list = get_all_files(idl_path)
    idl_file_list = []
    for file in file_list:
        if os.path.splitext(file)[-1] == ".idl":
            idl_file_list.append(file)
    return idl_file_list



class Test:
    def __init__(self, name, working_dir):
        self.name = name
        self.working_dir = working_dir
        self.idl_dir = os.path.join(self.working_dir, "foo")
        self.output_dir = os.path.join(working_dir, "out")
        self.target_dir = os.path.join(working_dir, "target")
        self.command = "../../hdi-gen -s full -m ipc -l cpp -r ohos.hdi:{} -d {}".format(working_dir, self.output_dir)

    def run(self):
        # please add test code here
        return False

    def run_success(self):
        self.add_idl_files()
        status, _ = exec_command(self.command)
        if status == 0 and compare_target_files(self.output_dir, self.target_dir):
            return True
        return False
    
    def run_fail(self):
        self.add_idl_files()
        status, _ = exec_command(self.command)

        expected_fail_output = ""
        with open(os.path.join(self.target_dir, "fail_output.txt"), 'r') as target_output:
            expected_fail_output = target_output.read()

        if status != 0 and expected_fail_output == _:
            return True
        return False

    def remove_output(self):
        exec_command("rm -rf {}".format(self.output_dir))
        return True

    def add_idl_files(self):
        idl_list = get_all_idl_files(self.idl_dir)
        for idl in idl_list:
            self.command = "".join((self.command, " -c {}".format(idl)))

    def test(self):
        print_success("[ RUN       ] {}".format(self.name))
        start_time = get_time_stamp()
        result = self.run()
        end_time = get_time_stamp()

        if result:
            print_success("[        OK ] {} ({}ms)".format(self.name, end_time - start_time))
        else:
            print_failure("[    FAILED ] {} ({}ms)".format(self.name, end_time - start_time))
        return result


# compile empty idl file
class UnitTest01(Test):
    def run(self):
        return self.run_fail()


# standard interface idl file
class UnitTest02(Test):
    def run(self):
        return self.run_success()


# standard callback idl file
class UnitTest03(Test):
    def run(self):
        return self.run_success()


# extended interface idl file
class UnitTest04(Test):
    def run(self):
        return self.run_success()


# interface with types idl file
class UnitTest05(Test):
    def run(self):
        return self.run_success()


# extended enum idl file
class UnitTest06(Test):
    def run(self):
        return self.run_success()


# extended struct idl file
class UnitTest07(Test):
    def run(self):
        return self.run_success()


# overload method idl file
class UnitTest08(Test):
    def run(self):
        return self.run_success()


# enum nesting idl file
class UnitTest09(Test):
    def run(self):
        return self.run_success()


class Tests:
    test_cases = [
        UnitTest01("UnitTestEmptyIdl", "01_empty_idl"),
        UnitTest02("UnitTestStandardInterface", "02_standard_interface_idl"),
        UnitTest03("UnitTestStandardCallback", "03_standard_callback_idl"),
        UnitTest04("UnitTestExtendedInterface", "04_extended_interface_idl"),
        UnitTest05("UnitTestTypesIdl", "05_types_idl"),
        UnitTest06("UnitTestEnumExtension", "06_extended_enum_idl"),
        UnitTest07("UnitTestStructExtension", "07_extended_struct_idl"),
        UnitTest08("UnitTestOverloadMethod", "08_overload_method_idl"),
        UnitTest09("UnitTestEnumNesting", "09_enum_nesting_idl"),
    ]

    @staticmethod
    def set_up_test_case():
        hdi_gen_file = "../../hdi-gen"
        ret = file_exists(hdi_gen_file)
        if not ret:
            hdi_gen_path = "../../"
            if make_binary_file(hdi_gen_path)[0] == 0:
                ret = True
        if not ret:
            print_failure("[===========] failed to make hdi-gen")
        return ret

    @staticmethod
    def tear_down_test_case():
        for case in Tests.test_cases:
            case.remove_output()
        hdi_gen_path = "../../"
        clean_binary_file(hdi_gen_path)

    @staticmethod
    def test():
        test_case_num = len(Tests.test_cases)
        success_case_num = 0
        print_success("[===========] start {} test".format(test_case_num))
        for test_case in Tests.test_cases:
            if test_case.test():
                success_case_num += 1
        print_success("[    PASSED ] {} test".format(success_case_num))
        failure_case_num = test_case_num - success_case_num
        if failure_case_num > 0:
            print_failure("[    FAILED ] {} test".format(failure_case_num))


if __name__ == "__main__":
    if not Tests.set_up_test_case():
        print_failure("test case set up failed!")
        exit(-1)
    Tests.test()
    Tests.tear_down_test_case()
