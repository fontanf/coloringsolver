import argparse
import sys
import os
import json
import pathlib
import shutil


def run_command(command):
    print(command)
    status = os.system(command)
    if status != 0:
        sys.exit(1)
    print()


parser = argparse.ArgumentParser(description='')
parser.add_argument(
        "algorithm",
        type=str,
        help='')
parser.add_argument(
        "output",
        type=str,
        help='')

args = parser.parse_args()


main = os.path.join(
        "install",
        "bin",
        "coloringsolver")
instances_path = os.path.join("data", "test_all.txt")

with open(args.output, 'w') as output_file:
    with open(instances_path, 'r') as instances_file:
        for line in instances_file:
            instance_base_path = line[:-1]
            instance_full_path = os.path.join("data", instance_base_path)
            certificate_full_path = f"{instance_full_path}_solution.txt"
            certificate_tmp_path = f"solution.txt"
            json_output_path = "output.json"
            command = (
                    main
                    + f"  --verbosity-level 1"
                    + f"  --input \"{instance_full_path}\""
                    " --format snap"
                    + f"  --algorithm {args.algorithm}"
                    + f"  --certificate \"{certificate_tmp_path}\""
                    + f"  --output \"{json_output_path}\""
                    + f"  --time-limit 1")
            run_command(command)

            # If solved to optimality, add to output file.
            json_output_file = open(json_output_path, "r")
            json_data = json.load(json_output_file)
            if json_data["Output"]["Value"] == "inf":
                continue
            bound = int(json_data["Output"]["Bound"])
            value = int(json_data["Output"]["Value"])
            if bound == value:
                output_file.write(f"{instance_base_path}\n")
                shutil.copyfile(
                        certificate_tmp_path,
                        certificate_full_path)
