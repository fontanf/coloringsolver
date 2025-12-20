import argparse
import gdown
import pathlib
import py7zr


parser = argparse.ArgumentParser(description='')
parser.add_argument(
        "-d", "--data",
        type=str,
        nargs='*',
        help='')
args = parser.parse_args()


if args.data is None:
    gdown.download(id="1oEIzXq16K8BwvbQo3ZE4mojX_rEGkG0g", output="data.7z")
    with py7zr.SevenZipFile("data.7z", mode="r") as z:
        z.extractall(path="data")
    pathlib.Path("data.7z").unlink()

if args.data is not None and "verma2015" in args.data:
    gdown.download(id="1EeQ5PQ4Sjq_P4VmuApq9ROTmq7qzM41D", output="data.7z")
    with py7zr.SevenZipFile("data.7z", mode="r") as z:
        z.extractall(path="data")
    pathlib.Path("data.7z").unlink()
