#!/usr/bin/env python3

import os
import posixpath
import subprocess
import sys
from pathlib import Path

import h5py

def compare_hdf5_structure(file1: Path, file2: Path) -> bool:
    def compare_groups(group1, group2):
        # Compare group keys
        if set(group1.keys()) != set(group2.keys()):
            return False

        # Compare attributes
        if set(group1.attrs.keys()) != set(group2.attrs.keys()):
            return False
        for key in group1.attrs.keys():
            if group1.attrs[key] != group2.attrs[key]:
                return False

        # Recursively compare sub-groups and datasets
        for key in group1.keys():
            item1 = group1[key]
            item2 = group2[key]
            if isinstance(item1, h5py.Group) and isinstance(item2, h5py.Group):
                if not compare_groups(item1, item2):
                    return False
            elif isinstance(item1, h5py.Dataset) and isinstance(item2, h5py.Dataset):
                if item1.shape != item2.shape or item1.dtype != item2.dtype:
                    return False
            else:
                return False
        return True

    with h5py.File(file1, 'r') as f1, h5py.File(file2, 'r') as f2:
        return compare_groups(f1, f2)


# get the directory of this script
test_dir = os.path.dirname(os.path.realpath(__file__))
# in the path replace \ with /, D:\ with /d/
test_dir = test_dir.replace("\\", "/")
# tell os.path.join to use / as the path separator
os.path.sep = "/"
exe = sys.argv[1]

print(f"test_dir: {test_dir}")
print(f"exe: {exe}")

fv_input_file = posixpath.join(test_dir, "SimID_1585623750_0_.fvinput")
vcg_input_file = posixpath.join(test_dir, "SimID_1585623750_0_.vcg")
output_file = posixpath.join(test_dir, "SimID_1585623750_0_.hdf5")
expected_output_file = posixpath.join(test_dir, "SimID_1585623750_0_.hdf5.expected")

if not posixpath.exists(exe):
    print(f"FiniteVolume_x64 executable {exe} not found. Exiting...")
    sys.exit(1)

if not posixpath.exists(fv_input_file):
    print(f".fvinput input file {fv_input_file} not found. Exiting...")
    sys.exit(1)

if not posixpath.exists(vcg_input_file):
    print(f".vcg input file {vcg_input_file} not found. Exiting...")
    sys.exit(1)

if not posixpath.exists(expected_output_file):
    print(f"Expected output file {expected_output_file} not found. Exiting...")
    sys.exit(1)

command = [exe, fv_input_file, vcg_input_file]
print(" ".join(command))

try:
    subprocess.check_call(command)
except subprocess.CalledProcessError:
    print("FiniteVolume_x64 failed to run. Exiting...")
    sys.exit(1)

# verify that the output files exist
if not os.path.isfile(output_file):
    print(f"Output file {output_file} not found. Exiting...")
    sys.exit(1)

# verify that the output files match the expected output files
if not compare_hdf5_structure(file1=Path(output_file), file2=Path(expected_output_file)):
    print(f"Output file {output_file} does not match expected output {expected_output_file}. Exiting...")
    sys.exit(1)

print("FiniteVolume_x64 solver completed and solution matched expected output. Exiting...")
sys.exit(0)
