#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2020-2021 Huawei Device Co., Ltd.
# 
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


import json
import os
import re
import ast
import platform
from string import Template

import hdf_utils
from hdf_tool_exception import HdfToolException
from .hdf_command_error_code import CommandErrorCode


class HdfLiteScan(object):
    def __init__(self, root, vendor, board):
        self.root = root
        self.vendor = vendor
        self.board = board

        self.build_path = hdf_utils.get_vendor_gn_path(self.root)
        if not os.path.exists(self.build_path):
            raise HdfToolException('file: %s not exist' % self.build_path,
                                   CommandErrorCode.TARGET_NOT_EXIST)

        self.framework_dir = hdf_utils.get_module_dir(self.root)
        if not os.path.exists(self.framework_dir):
            raise HdfToolException('file: %s not exist' % self.framework_dir,
                                   CommandErrorCode.TARGET_NOT_EXIST)

        self.hcs_path = hdf_utils.get_hcs_file_path(self.root, vendor, board)
        if not os.path.exists(self.hcs_path):
            raise HdfToolException('file: %s not exist' % self.hcs_path,
                                   CommandErrorCode.TARGET_NOT_EXIST)

        dot_file_list = hdf_utils.get_dot_configs_path(root, vendor, board)
        self.dot_file = list(filter(
            lambda x: x.endswith("debug_tee.config"), dot_file_list))
        self.contents = hdf_utils.read_file_lines(self.build_path)
        self.contents_config_enable = hdf_utils.read_file_lines(
            self.dot_file[0])
        self.re_temp = r"^modules"
        self.re_temp_model = r'"[a-z 0-9]+'
        self.re_temp_module_switch = r"^module_switch"
        self.re_temp_loscfg = r"LOSCFG[_ A-Z 0-9]+"
        self.re_temp_sources = "^sources"
        self.re_source_file1 = r"\$[A-Z _ / a-z 0-9 .]+\.c"
        self.re_source_file2 = r"[_ / a-z 0-9]+\.c"
        self.re_temp_header = r"^hdf_driver\(module_name\)"
        self.re_temp_if = r"^if"
        self.re_left_macro = r'^[A-Z _ 0-9]+'
        self.re_right_macro = r'\$[A-Z _ 0-9]+'

    def scan_build(self):
        start_index = 0
        end_index = 0
        state = 0
        for index, line in enumerate(self.contents):
            if re.compile(self.re_temp).match(line.strip()):
                start_index = index
                state += 1
            elif line.strip() == "[" and start_index > 0:
                state += 1
            elif line.strip() == "]" and start_index > 0:
                state -= 1
                if state == 0:
                    end_index = index + 1

        model_list = []
        for i in self.contents[start_index + 1: end_index - 1]:
            model_name = re.compile(self.re_temp_model).match(i.strip())
            if model_name:
                model_list.append(i.strip())
        return list(set(model_list))

    def _get_model_file_dict(self, liteos_model):
        parent_path_model = os.path.sep.join(
            self.build_path.split(os.path.sep)[:-1])
        model_path_dict = {}
        for model_path in liteos_model:
            model_file_path = os.path.join(parent_path_model, model_path[1:-2])
            model_name = model_path[1:-2].split("/")[0]
            if model_path_dict.get(model_name, None):
                model_path_dict.get(model_name).append(model_file_path)
            else:
                model_path_dict[model_name] = [model_file_path]
        return model_path_dict

    def model_able_scan(self, model_path_dict):
        for model_name, value in model_path_dict.items():
            for path in value:
                model_build = os.path.join(path, "BUILD.gn")
                if not os.path.exists(model_build):
                    continue
                model_build_lines = hdf_utils.read_file_lines(model_build)
                model_path_dict = self.model_build_file_able(
                    model_build_lines, model_path_dict, model_name, path)
        return model_path_dict

    def model_build_file_able(self, model_build_lines, model_path_dict,
                              model_name, path):
        status = True
        for line in model_build_lines:
            if re.search(self.re_temp_module_switch, line) and status:
                enable_key = '%s=y\n' % re.search(
                    self.re_temp_loscfg, line.strip()).group()
                if enable_key not in self.contents_config_enable:
                    model_path_dict[model_name].remove(path)
                else:
                    status = False
        return model_path_dict

    def get_need_result_only(self, model_path_dict, name,
                             import_gni_path, return_dict, need_replace):
        parent = model_path_dict[name][0]
        model_build = os.path.join(parent, "BUILD.gn")
        if os.path.exists(model_build):
            model_build_lines = hdf_utils.read_file_lines(model_build)
            key_index = self.get_index(model_config_lines=model_build_lines)
            enable_path_dict = self.enable_dict(
                index_list=key_index,
                enable_lines=model_build_lines,
                path=parent)
            replace_path = {}
            import_gni_path, replace_path, temp_name = self.build_able_analyze(
                model_build_lines, replace_path, import_gni_path)
            enable_path_dict[temp_name] = enable_path_dict.pop("temp")
            return_dict[name] = enable_path_dict
            need_replace[name] = replace_path
        return need_replace, return_dict, enable_path_dict, import_gni_path

    def build_able_analyze(self, model_build_lines, replace_path, import_gni_path):
        for index, line in enumerate(model_build_lines):
            if re.search(self.re_temp_module_switch, line.strip()):
                temp_name = re.search(self.re_temp_loscfg,
                                      line.strip()).group()
            elif re.search(self.re_left_macro, line.strip()):
                if line.strip().endswith("="):
                    temp_str = line.strip() + \
                               model_build_lines[index + 1].strip()
                    replace_path = self.build_line_analyze(temp_str, replace_path)
                else:
                    temp_str = line.strip()
                    replace_path = self.build_line_analyze(temp_str, replace_path)
            elif re.search(r"^import", line.strip()):
                import_gni_path.append(line.strip())
        return import_gni_path, replace_path, temp_name

    def build_line_analyze(self, temp_str, replace_path):
        if re.search(self.re_right_macro, temp_str):
            temp = re.search(
                self.re_right_macro, temp_str).group()
            temp_re_list = re.sub(
                self.re_right_macro,
                '${' + temp[1:] + "}",
                temp_str).split("=")
            replace_path[temp_re_list[0].strip()] = \
                temp_re_list[-1].strip().strip("\"")
        else:
            temp_split_list = temp_str.split("=")
            replace_path[temp_split_list[0].strip()] = \
                temp_split_list[-1].strip().strip("\"")
        return replace_path

    def get_need_result_list(self, model_path_dict, name, import_replace_name,
                             import_gni_path, return_dict, need_replace):
        multiple_path = {}
        for path in model_path_dict[name]:
            model_build = os.path.join(path, "BUILD.gn")
            if not os.path.exists(model_build):
                continue
            model_build_lines = hdf_utils.read_file_lines(model_build)
            key_index = self.get_index(
                model_config_lines=model_build_lines)
            replace_path = {}
            enable_path_dict = self.enable_dict(
                index_list=key_index,
                enable_lines=model_build_lines,
                path=path
            )
            for index, line in enumerate(model_build_lines):
                if re.search(self.re_temp_module_switch, line.strip()):
                    temp_name = re.search(self.re_temp_loscfg,
                                          line.strip()).group()
                elif re.search(self.re_left_macro, line.strip()):
                    replace_path = self.get_macro_replace_path(
                        line, model_build_lines,
                        index, replace_path, import_replace_name)
                elif re.search(r"^import", line.strip()):
                    import_gni_path.append(line.strip())
            enable_path_dict[temp_name] = enable_path_dict.pop("temp")
            multiple_path.update(enable_path_dict)
            if need_replace.get(name):
                need_replace[name].update(replace_path)
            else:
                need_replace[name] = replace_path
        return_dict[name] = multiple_path
        return need_replace, return_dict, enable_path_dict, import_gni_path

    def get_macro_replace_path(self, line, model_build_lines,
             index, replace_path, import_replace_name):
        if line.strip().endswith("="):
            temp_str = line.strip() + \
                       model_build_lines[index + 1].strip()
            replace_path = self.build_line_analyze(
                temp_str, replace_path)
        else:
            temp_str = line.strip()
            if re.search(self.re_right_macro, temp_str):
                temp = re.search(self.re_right_macro,
                                 temp_str).group()
                temp_re_list = re.sub(self.re_right_macro,
                                      '${' + temp[1:] + "}",
                                      temp_str).split("=")
                import_replace_name.add(temp[1:])
                replace_path[temp_re_list[0].strip()] = \
                    temp_re_list[-1].strip().strip("\"")
        return replace_path

    def get_need_result(self, model_path_dict):
        return_dict_temp = {}
        need_replace_temp = {}
        import_gni_path = []
        import_replace_name = set([])
        for name in list(model_path_dict.keys()):
            if len(model_path_dict[name]) == 0:
                pass
            else:
                if len(model_path_dict[name]) == 1:
                    need_replace, return_dict, \
                    enable_path_dict, import_gni_path = \
                        self.get_need_result_only(
                            model_path_dict, name,
                            import_gni_path, return_dict_temp, need_replace_temp)
                else:
                    need_replace, return_dict, \
                    enable_path_dict, import_gni_path = \
                        self.get_need_result_list(
                            model_path_dict, name, import_replace_name,
                            import_gni_path, return_dict_temp, need_replace_temp)

        for k_name in list(return_dict.keys()):
            for model_detail in list(return_dict.get(k_name).keys()):
                enable_key = '%s=y\n' % model_detail
                if enable_key not in self.contents_config_enable:
                    del return_dict.get(k_name)[model_detail]
                    continue
                if not return_dict.get(k_name).get(model_detail):
                    del return_dict.get(k_name)[model_detail]
                else:
                    return_dict = self.format_replace_string(
                        return_dict, model_name=k_name,
                        model_detail=model_detail)
        return return_dict, need_replace, \
               import_gni_path, import_replace_name

    def format_replace_string(self, return_dict, model_name, model_detail):
        temp_list = []
        for info in return_dict[model_name][model_detail]['drivers_sources']:
            if re.search(self.re_right_macro, info):
                temp = re.search(self.re_right_macro, info).group()
                temp_re_str = re.sub(self.re_right_macro,
                                   '${' + temp[1:] + "}", info)
                temp_list.append(temp_re_str.strip().strip("\""))
            else:
                temp_list.append(info.strip())
        return_dict[model_name][model_detail]['drivers_sources'] = temp_list
        return return_dict

    def get_index(self, model_config_lines):
        index_list = []
        count = 0
        for index, line in enumerate(model_config_lines):
            if re.search(self.re_temp_header, line.strip())\
                    or re.search(self.re_temp_if, line.strip()):
                count += 1
                index_list.append(index)
            elif line.strip() == "}":
                count -= 1
                if count == 0:
                    index_list.append(index)
        return index_list

    def enable_dict(self, index_list, enable_lines, path):
        config_name = ['BUILD.gn', 'Kconfig', 'Makefile']
        driver_configs = []
        result_dict = {}
        for file_name in os.listdir(path):
            if file_name in config_name:
                driver_configs.append(os.path.join(path, file_name))
        for index_num in range(len(index_list) - 1):
            temp_dict = {}
            header_name, temp_list = self.get_header_list(
                enable_lines, index_list, index_num)
            if header_name is not None:
                if header_name.find("if") != -1:
                    enable_name = re.search(
                        self.re_temp_loscfg, header_name).group()
                else:
                    enable_name = "temp"
            else:
                enable_name = "temp"
            temp_res_list = self.eval_source_str_path(temp_list)
            replace_finish = []
            for temp_path in temp_res_list:
                if temp_path.startswith('$'):
                    replace_finish.append(temp_path)
                else:
                    replace_finish.append(os.path.join(path, temp_path))
            temp_dict["driver_configs"] = driver_configs
            temp_dict["drivers_sources"] = replace_finish
            result_dict[enable_name] = temp_dict
        return result_dict

    def eval_source_str_path(self, temp_list):
        temp_res_list = []
        for temp_info in temp_list:
            source_file_path1 = re.search(
                self.re_source_file1, temp_info.strip())
            source_file_path2 = re.search(
                self.re_source_file2, temp_info.strip())
            if source_file_path1:
                temp_res_list.append(source_file_path1.group())
            elif source_file_path2:
                list1 = temp_info.strip().split("=")
                try:
                    str1 = ast.literal_eval(list1[-1].strip())[0]
                except NameError:
                    continue
                finally:
                    pass
                temp_res_list.append(re.search(
                    self.re_source_file2, str1.strip()).group())
        return temp_res_list

    def get_header_list(self, enable_lines, index_list, index_num):
        count = 0
        temp_list = []
        header_name = None
        for line in enable_lines[index_list[index_num]: index_list[index_num + 1]]:
            if re.search(self.re_temp_sources, line.strip()):
                if line.strip().find("+=") > 0 and \
                        line.strip().endswith("]"):
                    header_name = enable_lines[index_list[index_num]].strip()
                    temp_list.append(line)
                    continue
                if line.strip().find("+=") > 0:
                    count += 1
                    header_name = enable_lines[index_list[index_num]].strip()
                else:
                    count += 1
            elif line.strip() == "]":
                count -= 1
                if count == 0:
                    break
            if count > 0:
                temp_list.append(line)
        return header_name, temp_list

    def replace_file_path(self, return_dict, import_gni_path,
                          import_replace_name, need_replace):
        test_re = r'\([/ a-z . - " _]+\)'
        import_path = []
        template_replace_dict = {}
        for i in list(set(import_gni_path)):
            temp_path = re.search(test_re, i).group()[2:-2]
            if os.path.exists(self.root + temp_path):
                self.found_import_file_path(
                    temp_path, import_path,
                    import_replace_name, template_replace_dict)
        temp_template = Template(json.dumps(need_replace))
        replace_result_dict = json.loads(temp_template.substitute(
            template_replace_dict))
        for key_model in list(return_dict.keys()):
            replace_result_dict[key_model].update(template_replace_dict)
            replace_dict = json.loads(json.dumps(
                replace_result_dict[key_model]).replace("//", '/'))
            temp_template1 = Template(
                json.dumps(return_dict[key_model]))
            temp_key_model = json.loads(temp_template1.
                                        substitute(replace_dict))
            temp_res_key_model = self.driver_source_filter(temp_key_model)
            return_dict[key_model] = temp_res_key_model
        return_dict["deconfig"] = self.dot_file[0]
        return_dict["build"] = self.build_path
        return_dict["hcs_path"] = self.hcs_path
        return json.dumps(return_dict, indent=4)

    def driver_source_filter(self, temp_key_model):
        for enable_name_lower in list(temp_key_model.keys()):
            if enable_name_lower.find("HDF") != -1:
                key_name = ''.join(["HDF", enable_name_lower.
                                   split("HDF")[-1]]).lower()
            else:
                key_name = enable_name_lower.lower()
            temp_key_model[key_name] = temp_key_model.pop(enable_name_lower)
            for file_path in temp_key_model[key_name]["drivers_sources"]:
                if not os.path.exists(file_path):
                    temp_key_model[key_name]["drivers_sources"].remove(file_path)
        return temp_key_model

    def get_model_scan(self):
        liteos_model = self.scan_build()
        model_path_dict = self._get_model_file_dict(liteos_model=liteos_model)
        model_path_dict = self.model_able_scan(model_path_dict)
        return_dict, need_replace, import_gni_path, import_replace_name =\
            self.get_need_result(model_path_dict)
        return self.replace_file_path(return_dict, import_gni_path,
                                      import_replace_name, need_replace)

    def found_import_file_path(self, temp_path, import_path,
                               import_replace_name, template_replace_dict):
        import_info = hdf_utils.read_file_lines(self.root + temp_path)
        for temp_import in import_info:
            if re.search(r"^import", temp_import.strip()):
                import_path.append(temp_import.strip("import")[2:-3])
        for import_element in import_path:
            import_info.extend(hdf_utils.
                               read_file_lines(self.root + import_element[1:]))
        for name in import_replace_name:
            for temp_import in import_info:
                re_replace = "^%s" % name
                if re.search(re_replace, temp_import.strip()):
                    replace_line = temp_import.strip().split("=")
                    template_replace_dict[replace_line[0].strip()] =\
                        self.root + replace_line[-1]\
                            .strip().strip('"')
                    break
