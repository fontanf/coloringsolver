import networkx as nx
import os

instances_path = os.path.join("data", "test_all.txt")

with open(instances_path, 'w') as output_file:

    print("Generate complete graph...")
    for n in [1, 2, 5, 10, 20, 50]:
        graph = nx.complete_graph(n)
        instance_base_path = os.path.join(
                "tests",
                "complete",
                f"complete_n{n}")
        instance_full_path = os.path.join("data", instance_base_path)
        if not os.path.exists(os.path.dirname(instance_full_path)):
            os.makedirs(os.path.dirname(instance_full_path))
        nx.write_edgelist(graph, instance_full_path, data=False)
        output_file.write(f"{instance_base_path}\n")

    print("Generate empty graph...")
    for n in [1, 2, 5, 10, 20, 50]:
        graph = nx.empty_graph(n)
        instance_base_path = os.path.join(
                "tests",
                "empty",
                f"empty_n{n}")
        instance_full_path = os.path.join("data", instance_base_path)
        if not os.path.exists(os.path.dirname(instance_full_path)):
            os.makedirs(os.path.dirname(instance_full_path))
        nx.write_edgelist(graph, instance_full_path, data=False)
        output_file.write(f"{instance_base_path}\n")

    print("Generate random graphs...")
    for n in [1, 2, 5, 10, 20, 50]:
        for p in [0.02, 0.05, 0.1, 0.2, 0.5]:
            for seed in range(8):
                graph = nx.fast_gnp_random_graph(n, p, seed)
                instance_base_path = os.path.join(
                        "tests",
                        "random",
                        f"random_n{n}_p{p}_s{seed}")
                instance_full_path = os.path.join("data", instance_base_path)
                if not os.path.exists(os.path.dirname(instance_full_path)):
                    os.makedirs(os.path.dirname(instance_full_path))
                nx.write_edgelist(graph, instance_full_path, data=False)
                output_file.write(f"{instance_base_path}\n")

    print("Generate random regular graphs...")
    for n in [1, 2, 5, 10, 20, 50]:
        for d in range(2, n, 2):
            for seed in range(8):
                graph = nx.random_regular_graph(d, n, seed)
                instance_base_path = os.path.join(
                        "tests",
                        "random_regular",
                        f"random_regular_n{n}_d{d}_s{seed}")
                instance_full_path = os.path.join("data", instance_base_path)
                if not os.path.exists(os.path.dirname(instance_full_path)):
                    os.makedirs(os.path.dirname(instance_full_path))
                nx.write_edgelist(graph, instance_full_path, data=False)
                output_file.write(f"{instance_base_path}\n")
