import argparse
import sys
import os

parser = argparse.ArgumentParser(description='')
parser.add_argument('directory')
parser.add_argument(
        "-t", "--tests",
        type=str,
        nargs='*',
        help='')

args = parser.parse_args()


main = os.path.join(
        "bazel-bin",
        "coloringsolver",
        "main")


greedy_data = [
        (os.path.join("gebremedhin2013", "msdoor.mtx"), "matrixmarket"),
        (os.path.join("gebremedhin2013", "ldoor.mtx"), "matrixmarket"),
        (os.path.join("gebremedhin2013", "shipsec1.mtx"), "matrixmarket"),
        (os.path.join("gebremedhin2013", "shipsec5.mtx"), "matrixmarket"),
        (os.path.join("gebremedhin2013", "pkustk11.mtx"), "matrixmarket"),
        (os.path.join("gebremedhin2013", "ct20stif.mtx"), "matrixmarket"),
        (os.path.join("gebremedhin2013", "pwtk.mtx"), "matrixmarket"),
        (os.path.join("gebremedhin2013", "pkustk13.mtx"), "matrixmarket"),
        (os.path.join("gebremedhin2013", "nasasrb.mtx"), "matrixmarket"),
        (os.path.join("gebremedhin2013", "bmw3_2.mtx"), "matrixmarket")]


if args.tests is None or "greedy-largest-first" in args.tests:
    print("Greedy, largest first")
    print("---------------------")
    print()

    for instance, instance_format in greedy_data:
        instance_path = os.path.join(
                "data",
                instance)
        json_output_path = os.path.join(
                args.directory,
                "greedy_largest_first",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + "  --format \"" + instance_format + "\""
                + "  --algorithm greedy"
                + " --ordering largest-first"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


if args.tests is None or "greedy-incidence-degree" in args.tests:
    print("Greedy, incidence degree")
    print("------------------------")
    print()

    for instance, instance_format in greedy_data:
        instance_path = os.path.join(
                "data",
                instance)
        json_output_path = os.path.join(
                args.directory,
                "greedy_incidence_degree",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + "  --format \"" + instance_format + "\""
                + "  --algorithm greedy"
                + " --ordering incidence-degree"
                + " --reverse 1"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


if args.tests is None or "greedy-smallest-last" in args.tests:
    print("Greedy, smallest last")
    print("---------------------")
    print()

    for instance, instance_format in greedy_data:
        instance_path = os.path.join(
                "data",
                instance)
        json_output_path = os.path.join(
                args.directory,
                "greedy_smallest_last",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + "  --format \"" + instance_format + "\""
                + "  --algorithm greedy"
                + " --ordering smallest-last"
                + " --reverse 1"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


if args.tests is None or "greedy-dynamic-largest-first" in args.tests:
    print("Greedy, dynamic largest first")
    print("-----------------------------")
    print()

    for instance, instance_format in greedy_data:
        instance_path = os.path.join(
                "data",
                instance)
        json_output_path = os.path.join(
                args.directory,
                "greedy_dynamic_largest_first",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + "  --format \"" + instance_format + "\""
                + "  --algorithm greedy"
                + " --ordering dynamic-largest-first"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()
