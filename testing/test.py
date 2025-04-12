import matplotlib.pyplot as plt
import networkx as nx
import re
import tkinter as tk
from tkinter import filedialog
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg


with open("./testing/Plik1.txt") as f:
    graph_lines = f.readlines()

with open("./testing/Plik2.txt") as f:
    partition_lines = f.readlines()


G = nx.Graph()
for line in graph_lines:
    match = re.match(r"Node (\d+)\(\d+\): ([\d ]+)", line)
    if match:
        node = int(match.group(1))
        neighbors = list(map(int, match.group(2).split()))
        for neighbor in neighbors:
            G.add_edge(node, neighbor)


node_to_partition = {}
partition_re = re.compile(r"Node (\d+) \(Partition (\d+)\):")
for line in partition_lines:
    match = partition_re.search(line)
    if match:
        node = int(match.group(1))
        partition = int(match.group(2))
        node_to_partition[node] = partition


def get_partition_colors():
    unique_parts = sorted(set(node_to_partition.values()))
    cmap = plt.cm.get_cmap('tab10', len(unique_parts))
    return {p: cmap(i) for i, p in enumerate(unique_parts)}

def draw_graph(color_by_partition=False):
    plt.clf()
    pos = nx.spring_layout(G, seed=42)

    if color_by_partition:
        
        H = nx.Graph()
        H.add_nodes_from(G.nodes())
        part_colors = get_partition_colors()

        for u, v in G.edges():
            if node_to_partition.get(u) == node_to_partition.get(v):
                H.add_edge(u, v)

        node_colors = [part_colors.get(node_to_partition.get(n, -1), (0.5, 0.5, 0.5)) for n in H.nodes()]
        nx.draw(H, pos, with_labels=True, node_color=node_colors, edge_color="#cccccc", node_size=300, font_size=8)
    else:
        
        node_colors = ['lightgray'] * len(G.nodes())
        nx.draw(G, pos, with_labels=True, node_color=node_colors, edge_color="#cccccc", node_size=300, font_size=8)

    canvas.draw()

root = tk.Tk()
root.title("Wizualizacja Grafu z Partycjami")

frame = tk.Frame(root)
frame.pack()

fig = plt.figure(figsize=(10, 8))
canvas = FigureCanvasTkAgg(fig, master=frame)
canvas.get_tk_widget().pack()

button_frame = tk.Frame(root)
button_frame.pack(pady=10)

btn_default = tk.Button(button_frame, text="Pokaż bez kolorów partycji", command=lambda: draw_graph(False))
btn_default.pack(side=tk.LEFT, padx=10)

btn_partition = tk.Button(button_frame, text="Pokaż z kolorami partycji", command=lambda: draw_graph(True))
btn_partition.pack(side=tk.LEFT, padx=10)

draw_graph(False)
root.mainloop()
