#!/usr/bin/env python

#
# This source file is part of Junkyard repository.
# Licensed under the terms of the GNU General Public License v3 or later.
# Copyright (C) Biswapriyo Nath
#
# create-apiset-def.py: Creates DEF and MRI files from a text file
# which is extracted from a .lib file. Open the .lib file using 7zip
# and extract 1.txt file.
# Options:
# --input: Full path of text file from lib.
# --output: Name of output library.
#

import argparse
import sys


def create_mri_file(mri_file_name: str, def_file_list: list[str]) -> None:
    if not mri_file_name or not def_file_list:
        return
    def_file_list.sort()
    def_file_list.insert(0, f"CREATE lib{mri_file_name}.a")
    def_file_list.extend(["SAVE", "END", ""])
    with open(f"{mri_file_name}.mri", mode="w", newline="\n") as mri_file:
        mri_file.write("\n".join(def_file_list))


# Unused, should be used in the function below
def create_def_file(def_file_name: str, api_list: list[str]) -> None:
    if not def_file_name or not api_list:
        return
    api_list.insert(0, f"LIBRARY {def_file_name}")
    api_list.insert(1, "EXPORTS")
    api_list.append("")
    with open(f"{def_file_name}.def", mode="w", newline="\n") as def_file:
        def_file.write("\n".join(api_list))


def parse_lib_text(lib_text_file_name: str, mri_file_name: str) -> None:
    api_list = []
    def_file_list = []
    def_file_name = ""
    def_file_name_temp = ""

    with open(lib_text_file_name) as file:
        lines = file.readlines()

    for line in lines:
        def_file_name_temp = line.split(".")[1].strip().lower()
        if def_file_name_temp != def_file_name:
            def_file_name = def_file_name_temp
            def_file_list.append(f"ADDLIB lib{def_file_name}.a")
            with open(def_file_name + ".def", mode="w", newline="\n") as def_file:
                def_file.write("LIBRARY " + def_file_name + "\n")
                def_file.write("EXPORTS\n")
        if "__imp_" in line:
            with open(def_file_name + ".def", mode="a", newline="\n") as def_file:
                def_file.write(line.split("__imp_")[1].strip() + "\n")

    create_mri_file(mri_file_name, def_file_list)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input", help="Full path of text file from lib", nargs=1, required=True
    )
    parser.add_argument(
        "--output", help="Name of output library", nargs=1, required=True
    )
    args = parser.parse_args()
    parse_lib_text(args.input[0], args.output[0])


if __name__ == "__main__":
    main()
