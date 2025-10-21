import argparse
import gdown
import os
import shutil
import pathlib


parser = argparse.ArgumentParser(description='')
parser.add_argument(
        "-d", "--data",
        type=str,
        nargs='*',
        help='')
args = parser.parse_args()


if args.data is None:
    gdown.download(id="1aZj423aiN_wYJkFLvLHYnFYDxeYFnEHZ", output="data.7z")
    os.system("7z x data.7z")
    pathlib.Path("data.7z").unlink()
    shutil.copytree("graph_coloring", "data", dirs_exist_ok=True)

if args.data is not None and "verma2015" in args.data:
    gdown.download(id="1yFyrbrX1V88DsiCN4SIKRTKun9t9RMgo", output="data.7z")
    os.system("7z x data.7z")
    pathlib.Path("data.7z").unlink()
    shutil.copytree("graph_coloring_verma2015", "data", dirs_exist_ok=True)
