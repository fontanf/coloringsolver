import os
import json
import sys

data       = sys.argv[1]
algorithm  = sys.argv[2]
timelimit  = sys.argv[3] if len(sys.argv) > 3 else None

datas = {}

datas["graphcoloring_s"] = [
        "graphcoloring/1-FullIns_3.col",
        "graphcoloring/1-Insertions_4.col",
        "graphcoloring/2-FullIns_3.col",
        "graphcoloring/2-Insertions_3.col",
        "graphcoloring/3-Insertions_3.col",
        "graphcoloring/anna.col",
        "graphcoloring/ash331GPIA.col",
        "graphcoloring/david.col",
        "graphcoloring/DSJC125.1.col",
        "graphcoloring/DSJR500.1.col",
        "graphcoloring/fpsol2.i.1.col",
        "graphcoloring/fpsol2.i.2.col",
        "graphcoloring/fpsol2.i.3.col",
        "graphcoloring/games120.col",
        "graphcoloring/homer.col",
        "graphcoloring/huck.col",
        "graphcoloring/inithx.i.1.col",
        "graphcoloring/inithx.i.2.col",
        "graphcoloring/inithx.i.3.col",
        "graphcoloring/jean.col",
        "graphcoloring/le450_25a.col",
        "graphcoloring/le450_25b.col",
        "graphcoloring/le450_5c.col",
        "graphcoloring/le450_5d.col",
        "graphcoloring/miles1000.col",
        "graphcoloring/miles1500.col",
        "graphcoloring/miles250.col",
        "graphcoloring/miles500.col",
        "graphcoloring/miles750.col",
        "graphcoloring/mug88_1.col",
        "graphcoloring/mug88_25.col",
        "graphcoloring/mulsol.i.1.col",
        "graphcoloring/mulsol.i.2.col",
        "graphcoloring/mulsol.i.3.col",
        "graphcoloring/mulsol.i.4.col",
        "graphcoloring/mulsol.i.5.col",
        "graphcoloring/myciel3.col",
        "graphcoloring/myciel4.col",
        "graphcoloring/myciel5.col",
        "graphcoloring/qg.order30.col",
        "graphcoloring/queen5_5.col",
        "graphcoloring/queen6_6.col",
        "graphcoloring/queen7_7.col",
        "graphcoloring/queen8_12.col",
        "graphcoloring/queen8_8.col",
        "graphcoloring/queen9_9.col",
        "graphcoloring/r1000.1.col",
        "graphcoloring/r125.1.col",
        "graphcoloring/r125.1c.col",
        "graphcoloring/r125.5.col",
        "graphcoloring/r250.1.col",
        "graphcoloring/r250.1c.col",
        "graphcoloring/school1.col",
        "graphcoloring/will199GPIA.col",
        "graphcoloring/zeroin.i.1.col",
        "graphcoloring/zeroin.i.2.col",
        "graphcoloring/zeroin.i.3.col",
]

datas["graphcoloring_m"] = [
        "graphcoloring/1-FullIns_4.col",
        "graphcoloring/2-FullIns_4.col",
        "graphcoloring/3-FullIns_3.col",
        "graphcoloring/4-FullIns_3.col",
        "graphcoloring/5-FullIns_3.col",
        "graphcoloring/4-Insertions_3.col",
        "graphcoloring/ash608GPIA.col",
        "graphcoloring/ash958GPIA.col",
        "graphcoloring/le450_15a.col",
        "graphcoloring/mug100_1.col",
        "graphcoloring/mug100_25.col",
        "graphcoloring/qg.order40.col",
        "graphcoloring/wap05a.col",
        "graphcoloring/myciel6.col",
        "graphcoloring/school1_nsh.col",
]

datas["graphcoloring_h"] = [
        "graphcoloring/flat300_28_0.col",
        "graphcoloring/r1000.5.col",
        "graphcoloring/r250.5.col",
        "graphcoloring/DSJR500.5.col",
        "graphcoloring/DSJR500.1c.col",
        "graphcoloring/DSJC125.5.col",
        "graphcoloring/DSJC125.9.col",
        "graphcoloring/DSJC250.9.col",
        "graphcoloring/queen10_10.col",
        "graphcoloring/queen11_11.col",
        "graphcoloring/queen12_12.col",
        "graphcoloring/queen13_13.col",
        "graphcoloring/queen14_14.col",
        "graphcoloring/queen15_15.col",
]

datas["graphcoloring_j"] = [
        "graphcoloring/le450_5a.col",
        "graphcoloring/le450_5b.col",
        "graphcoloring/le450_15b.col",
        "graphcoloring/le450_15c.col",
        "graphcoloring/le450_15d.col",
        "graphcoloring/le450_25c.col",
        "graphcoloring/le450_25d.col",
        "graphcoloring/myciel7.col",
        "graphcoloring/1-FullIns_5.col",
        "graphcoloring/2-FullIns_4.col",
        "graphcoloring/2-FullIns_5.col",
        "graphcoloring/3-FullIns_4.col",
        "graphcoloring/4-FullIns_4.col",
        "graphcoloring/5-FullIns_4.col",
        "graphcoloring/1-Insertions_5.col",
        "graphcoloring/1-Insertions_6.col",
        "graphcoloring/2-Insertions_4.col",
        "graphcoloring/2-Insertions_5.col",
        "graphcoloring/3-Insertions_4.col",
        "graphcoloring/3-Insertions_5.col",
        "graphcoloring/4-Insertions_3.col",
        "graphcoloring/4-Insertions_4.col",
        "graphcoloring/qg.order60.col",
        "graphcoloring/qg.order100.col",
        "graphcoloring/DSJC250.1.col",
        "graphcoloring/DSJC250.5.col",
        "graphcoloring/DSJC500.1.col",
        "graphcoloring/DSJC500.5.col",
        "graphcoloring/DSJC500.9.col",
        "graphcoloring/DSJC1000.1.col",
        "graphcoloring/DSJC1000.5.col",
        "graphcoloring/DSJC1000.9.col",
        "graphcoloring/flat1000_50_0.col",
        "graphcoloring/flat1000_60_0.col",
        "graphcoloring/flat1000_76_0.col",
        "graphcoloring/r1000.1c.col",
        "graphcoloring/abb313GPIA.col",
        "graphcoloring/latin_square_10.col",
        "graphcoloring/wap01a.col",
        "graphcoloring/wap02a.col",
        "graphcoloring/wap03a.col",
        "graphcoloring/wap04a.col",
        "graphcoloring/wap06a.col",
        "graphcoloring/wap07a.col",
        "graphcoloring/wap08a.col",
        "graphcoloring/C2000.5.col",
        "graphcoloring/C4000.5.col",
        # "graphcoloring/C2000.9.col",
]

if __name__ == "__main__":

    directory_in = "data"
    directory_out = os.path.join("output", algorithm + (" | " + str(timelimit) if timelimit != None else ""), data)
    if not os.path.exists(directory_out):
        os.makedirs(directory_out)

    results_file = open(os.path.join(directory_out, "results.csv"), "w")
    results_file.write("Instance,Value,Time to best,Time to end\n")

    main_exec = os.path.join(".", "bazel-bin", "coloringsolver", "main")
    for instance_name in datas[data]:
        instance_path = os.path.join(directory_in, instance_name)
        output_path   = os.path.join(directory_out, instance_name + ".json")
        cert_path     = os.path.join(directory_out, instance_name + "_solution.txt")
        if not os.path.exists(os.path.dirname(output_path)):
            os.makedirs(os.path.dirname(output_path))

        command = main_exec \
                + " -v" \
                + " -i \"" + instance_path + "\"" \
                + (" -t " + str(timelimit) if timelimit != None else "") \
                + " -a \"" + algorithm + "\"" \
                + " -c \"" + cert_path + "\"" \
                + " -o \"" + output_path + "\""
        print(command)
        os.system(command)
        print()

        output_file = open(output_path, "r")
        d = json.load(output_file)

        k = 0
        while "Solution" + str(k + 1) in d.keys():
            k += 1

        results_file.write(instance_name \
                + "," + str(d["Solution"]["Value"]) \
                + "," + str(d["Solution" + str(k)]["Time"]) \
                + "," + str(d["Solution"]["Time"]) \
                + "\n")

