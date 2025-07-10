#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2022 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


import argparse
import sys
import json
import os
import stat


def collect_value_in_dict(val):
    res = []
    if isinstance(val, dict):
        for item_val in val.values():
            res += collect_value_in_dict(item_val)
    elif isinstance(val, list):
        res += val
    elif isinstance(val, str):
        res.append(val)
    return res


def add_string_to_pool(all_metas, string_pool, locale_id, resource_id, val):
    locale_metas = all_metas.get(locale_id)
    if locale_metas is None or (not isinstance(locale_metas, dict)):
        return all_metas, string_pool
    locale_metas[resource_id] = collect_value_in_dict(val)

    resource_str = '_'.join(locale_metas.get(resource_id, []))
    if resource_str.endswith('٪\u061c'):
        resource_str = "{}{}".format(resource_str, '\u200f')
    if string_pool.find(resource_str) == -1:
        string_pool = "{}{}".format(string_pool, resource_str)
    return all_metas, string_pool


def generate_all_metas(path):
    file_name_list = os.listdir(path)
    locales_file_name = 'locales.json'
    if locales_file_name not in file_name_list:
        print('can not find ', locales_file_name)
    else:
        file_name_list.remove(locales_file_name)

    resource_item_file_name = 'resource_items.json'
    if resource_item_file_name not in file_name_list:
        print('can not find ', resource_item_file_name)
    else:
        file_name_list.remove(resource_item_file_name)

    all_metas = {}
    with open(os.path.join(path, locales_file_name), 'r', encoding='utf-8') as locale_file:
        locales_content = locale_file.read()
        locales_json = json.loads(locales_content)
        for locale in locales_json['locales']:
            all_metas[locale] = {}

    with open(os.path.join(path, resource_item_file_name), 'r', encoding='utf-8') as resource_file:
        resource_item_content = resource_file.read()
        resource_item_json = json.loads(resource_item_content)

    string_pool = ""
    for resource_name, resource_id in resource_item_json.items():
        resource_name = "{}{}".format(resource_name, '.json')
        with open(os.path.join(path, resource_name), 'r', encoding='utf-8') as item_file:
            resource_content = item_file.read()
            resource_json = json.loads(resource_content)
            for locale_id, val in resource_json.items():
                all_metas, string_pool = add_string_to_pool(all_metas, string_pool, locale_id, resource_id, val)

    return all_metas, string_pool


def compute_header_bts(locales_num, meta_num):
    file_hash2bts = int('0').to_bytes(1, 'big') * 4
    version2bts = int('1').to_bytes(1, 'big')
    reserved2bts = int('0').to_bytes(1, 'big') * 5

    header_size = 16
    locale_index_size = 8
    meta_data_size = 6
    meta_offset = header_size + locale_index_size * locales_num
    meta_offset2bts = meta_offset.to_bytes(2, 'big')
    locales_num2bts = locales_num.to_bytes(2, 'big')
    string_pool_offset2bts = (meta_offset + meta_data_size * meta_num).to_bytes(2, 'big')

    header_bts = file_hash2bts + version2bts + reserved2bts + meta_offset2bts + locales_num2bts + \
        string_pool_offset2bts
    return header_bts


def get_mask(locale):
    locale_parts = locale.split('-')

    langs = ""
    char_off = 48
    lang_first_begin = 25
    lang_second_begin = 18
    if locale_parts[0] == 'fil':
        langs = 'tl'
    elif locale_parts[0] == 'mai':
        langs = 'md'
    elif locale_parts[0] == 'he':
        langs = 'iw'
    elif locale_parts[0] == 'id':
        langs = 'in'
    else:
        langs = locale_parts[0]
    mask = (ord(langs[0]) - char_off) << lang_first_begin | (ord(langs[1]) - char_off) << lang_second_begin

    script_value = 0
    script_begin = 14
    if len(locale_parts) >= 2 and len(locale_parts[1]) > 2:
        if locale_parts[1] == 'Latn':
            script_value = 0x1
        elif locale_parts[1] == 'Hans':
            script_value = 0x2
        elif locale_parts[1] == 'Hant':
            script_value = 0x3
        elif locale_parts[1] == 'Qaag':
            script_value = 0x4
        elif locale_parts[1] == 'Cyrl':
            script_value = 0x5
        elif locale_parts[1] == 'Deva':
            script_value = 0x6
        elif locale_parts[1] == 'Guru':
            script_value = 0x7    
        mask = mask | script_value << script_begin

    region = ''
    region_first_begin = 7
    if len(locale_parts) == 3:
        region = locale_parts[2]
    elif len(locale_parts) == 2 and len(locale_parts[1]) == 2:
        region = locale_parts[1]
    if region != '':
        mask = mask | (ord(region[0]) - char_off) << region_first_begin | (ord(region[1]) - char_off)

    return mask.to_bytes(4, 'big')


def content2bytes(all_metas, string_pool, dat_save_path):
    meta_num = 0
    for key, val in all_metas.items():
        meta_num += len(val.keys())
    locale_num = len(all_metas.keys())
    header_bts = compute_header_bts(locale_num, meta_num)

    string_pool2bts = string_pool.encode(encoding='utf-8', errors='strict')
    meta_acc = 0
    header_size = 16
    locale_index_size = 8
    meta_data_size = 6
    meta_offset = header_size + locale_index_size * locale_num
    total_locale_index2bts = bytes()
    total_meta2bts = bytes()
    for key, val in all_metas.items():
        locale_mask2bts = get_mask(key)
        locale_meta_offset = meta_offset + meta_data_size * meta_acc
        locale_meta_num = len(val.keys())
        meta_acc += locale_meta_num
        locale_index2bts = locale_mask2bts + locale_meta_offset.to_bytes(2, 'big') + locale_meta_num.to_bytes(2, 'big')
        total_locale_index2bts += locale_index2bts

        for resource_id, resource_vals in val.items():
            resource_str = '_'.join(resource_vals)
            if resource_str.endswith('٪\u061c'):
                resource_str = "{}{}".format(resource_str, '\u200f')
            resource_str2bts = resource_str.encode(encoding='utf-8', errors='strict')
            meta2bts = resource_id.to_bytes(2, 'big')
            meta_offset_in_pool = string_pool2bts.find(resource_str2bts)
            meta2bts += meta_offset_in_pool.to_bytes(2, 'big')
            meta2bts += len(resource_str2bts).to_bytes(2, 'big')
            total_meta2bts += meta2bts

    bts = header_bts + total_locale_index2bts + total_meta2bts + string_pool2bts
    flags = os.O_WRONLY
    modes = stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IROTH
    with os.fdopen(os.open(dat_save_path, flags, modes), 'wb') as dat_file:
        dat_file.write(bts)


if __name__ == '__main__':
    current_file_path = os.path.abspath(__file__)
    resource_path = os.path.join(os.path.dirname(current_file_path), "..", "..", "..", "..", "resource")
    metas, pool = generate_all_metas(resource_path)

    up_path = [".."] * 8
    save_path = [os.path.dirname(current_file_path)] + up_path + ["frameworks", "i18n", "i18n.dat"]
    save_path = os.path.join(*save_path)
    content2bytes(metas, pool, save_path)
