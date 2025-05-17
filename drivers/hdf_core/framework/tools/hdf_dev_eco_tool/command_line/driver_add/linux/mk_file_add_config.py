#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2022-2023 Huawei Device Co., Ltd.
#
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


import os
import re
from string import Template

import hdf_utils
from ..liteos.gn_file_add_config import analyze_parent_path


def find_makefile_file_end_index(date_lines, model_name):
    file_end_flag = "ccflags-y"
    end_index = 0
    model_dir_name = ("%s_ROOT_DIR" % model_name.upper())
    model_dir_value = ""

    for index, line in enumerate(date_lines):
        if line.startswith("#"):
            continue
        elif line.strip().startswith(file_end_flag):
            end_index = index
            break
        elif line.strip().startswith(model_dir_name):
            model_dir_value = line.split("=")[-1].strip()
        else:
            continue
    result_tuple = (end_index, model_dir_name, model_dir_value)
    return result_tuple


def formate_mk_config_build(source_path, date_lines, devices, root):
    if len(source_path) > 1:
        sources_line = []
        obj_first_line = "\nobj-$(CONFIG_DRIVERS_HDF_${model_name_upper}_${driver_name_upper}) += \\\n"
        temp_line = "\t\t\t\t$(${file_parent_path})/${source_path}.o"
        for source in source_path:
            temp_handle = Template(temp_line.replace("$(", "temp_flag"))
            temp_dict = analyze_parent_path(
                date_lines, source, "", devices, root)
            try:
                temp_dict['source_path'] = temp_dict.get(
                    'source_path').strip(".c")
            except KeyError as _:
                continue
            finally:
                pass
            if source == source_path[-1]:
                sources_line.append("".join([temp_handle.substitute(
                    temp_dict).replace("temp_flag", "$("), "\n"]))
            else:
                sources_line.append("".join([temp_handle.substitute(
                    temp_dict).replace("temp_flag", "$("), " \\", "\n"]))
        build_resource = obj_first_line + "".join(sources_line)
    else:
        build_resource = "LOCAL_SRCS += $(${file_parent_path})/${source_path}\n"
        for source in source_path:
            temp_handle = Template(build_resource.replace("$(", "temp_flag"))
            temp_dict = analyze_parent_path(
                date_lines, source, "", devices, root)
            build_resource = temp_handle.substitute(temp_dict).replace("temp_flag", "$(")
    return build_resource


def audio_makefile_template(date_lines, driver, source_path, head_path, devices, root):
    judge_result = judge_driver_config_exists(date_lines, driver_name=driver)
    if judge_result:
        return
    sources_lines = []
    obj_first_line = "\nobj-$(CONFIG_DRIVERS_HDF_${model_name_upper}_${driver_name_upper}) += \\\n"
    temp_line = "        ${source_path}.o"
    for source in source_path:
        temp_handle = Template(temp_line.replace("$(", "temp_flag"))
        temp_dict = analyze_parent_path(
            date_lines, source, "", devices, root)
        try:
            temp_dict['source_path'] = temp_dict.get(
                'source_path').strip(".c")
        except KeyError as _:
            continue
        finally:
            pass
        if source == source_path[-1]:
            sources_lines.append(temp_handle.substitute(temp_dict).replace("temp_flag", "$("))
        else:
            temp_replace_str = temp_handle.substitute(temp_dict).replace("temp_flag", "$(")
            sources_lines.append("{} \\\n".format(temp_replace_str))
    build_resource = "{}{}".format(obj_first_line, "".join(sources_lines))
    head_line = []
    ccflags_first_line = "\nccflags-$(CONFIG_DRIVERS_HDF_${model_name_upper}_${driver_name_upper}) += \\\n"
    temp_line = "        -I$(srctree)/$(${file_parent_path})/${head_path}"
    for head_file in head_path:
        temp_handle = Template(temp_line.replace("$(", "temp_flag"))
        temp_dict = analyze_parent_path(
            date_lines, "", head_file, devices, root)
        if head_file == head_path[-1]:
            temp_str = "".join([temp_handle.substitute(
                temp_dict).replace("temp_flag", "$("), "\n"])
        else:
            temp_str = "".join([temp_handle.substitute(
                temp_dict).replace("temp_flag", "$("), " \\", "\n"])
        head_line.append(temp_str)
    build_head = ccflags_first_line + "".join(head_line)
    makefile_add_template = build_resource + build_head
    return makefile_add_template


def audio_linux_makefile_operation(path, args_tuple):
    source_path, head_path, module, driver, root, devices, board = args_tuple
    makefile_gn_path = path
    mk_re_str = r"CONFIG_DRIVERS_HDF_(?P<name>.+[A-Z0-9])"
    flag_re = r"obj"
    sign_str = "(KHDF_AUDIO_BASE_ROOT_DIR)/device/board/"
    temp_file = hdf_utils.MakefileAndKconfigFileParse(
        file_path=makefile_gn_path, flag_str=flag_re, re_name=mk_re_str)
    board_makefile_dict = {}
    for block_info in temp_file.split_target_block(flag_re):
        if block_info.find(sign_str) > 0:
            board_name = re.search(mk_re_str, block_info).group()
            board_makefile_dict[board_name] = re.findall(
                r"\(KHDF_AUDIO_BASE_ROOT_DIR\)/device/board/[a-z0-9_/]+", block_info)[0]
    if board.startswith("rk3568"):
        parent_str = board_makefile_dict.get("CONFIG_DRIVERS_HDF_AUDIO_RK3568")
    elif board.startswith("hispark_taurus"):
        parent_str = board_makefile_dict.get("CONFIG_DRIVERS_HDF_AUDIO_HI3516CODEC")
    temp_makefile = "/".join(parent_str.split("/")[1:])
    makefile_path = os.path.join(root, temp_makefile, "Makefile")
    date_lines = hdf_utils.read_file_lines(makefile_path)
    makefile_add_template = audio_makefile_template(
        date_lines, driver, source_path, head_path, devices, root)

    temp_handle = Template(makefile_add_template.replace("$(", "temp_flag"))
    temp_replace = {
        'model_name_upper': module.upper(),
        'driver_name_upper': driver.upper(),
    }
    new_line = temp_handle.substitute(temp_replace).replace("temp_flag", "$(")
    date_lines = date_lines + [new_line]
    hdf_utils.write_file_lines(makefile_path, date_lines)
    return makefile_path


def judge_driver_config_exists(date_lines, driver_name):
    for _, line in enumerate(date_lines):
        if line.startswith("#"):
            continue
        elif line.find(driver_name) != -1:
            return True
    return False


def linux_makefile_operation(path, driver_file_path, head_path, module, driver):
    makefile_gn_path = path
    date_lines = hdf_utils.read_file_lines(makefile_gn_path)
    source_file_path = driver_file_path.replace('\\', '/')
    result_tuple = find_makefile_file_end_index(date_lines, model_name=module)
    judge_result = judge_driver_config_exists(date_lines, driver_name=driver)
    if judge_result:
        return
    end_index, model_dir_name, model_dir_value = result_tuple

    if module == "sensor":
        first_line = "\nobj-$(CONFIG_DRIVERS_HDF_${model_name_upper}_${driver_name_upper}) += \\\n"
        second_line = "              $(SENSOR_ROOT_CHIPSET)/${source_file_path}\n"
        third_line = "\n"
        makefile_add_template = first_line + second_line + third_line
        temp_handle = Template(makefile_add_template.replace("$(", "temp_flag"))
        str_start = source_file_path.find(module) + len(module) + 1
        temp_replace = {
            'model_name_upper': module.upper(),
            'driver_name_upper': driver.upper(),
            'source_file_path': source_file_path[str_start:].replace(".c", ".o")
        }
    else:
        first_line = "\nobj-$(CONFIG_DRIVERS_HDF_${model_name_upper}_${driver_name_upper}) += \\\n"
        second_line = "              $(${model_name_upper}_ROOT_DIR)/${source_file_path}\n"
        third_line = "ccflags-y += -I$(srctree)/drivers/hdf/framework/model/${head_file_path}\n"
        makefile_add_template = first_line + second_line + third_line
        include_model_info = model_dir_value.split("model")[-1].strip('"') + "/"
        makefile_path_config = source_file_path.split(include_model_info)
        temp_handle = Template(makefile_add_template.replace("$(", "temp_flag"))
        temp_replace = {
            'model_name_upper': module.upper(),
            'driver_name_upper': driver.upper(),
            'source_file_path': makefile_path_config[-1].replace(".c", ".o"),
            'head_file_path': '/'.join(head_path.split("model")[-1].strip(os.path.sep).split(os.path.sep)[:-1])
        }
    new_line = temp_handle.substitute(temp_replace).replace("temp_flag", "$(")
    endif_status = False
    for index, v in enumerate(date_lines[::-1]):
        if v.strip().startswith("endif"):
            endif_status = True
            end_line_index = len(date_lines) - index - 1
            date_lines = date_lines[:end_line_index] + [new_line] + [date_lines[end_line_index]]
            break
    if not endif_status:
        date_lines = date_lines + [new_line]
    hdf_utils.write_file_lines(makefile_gn_path, date_lines)
