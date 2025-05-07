import networkx as nx
import matplotlib.pyplot as plt
import re
import numpy as np

# 1) Utwórz pusty graf
G = nx.Graph()

# 2) Wczytaj dane z pliku
with open("./testing/podgrafy.txt", encoding="utf-8") as f:
    for line in f:
        line = line.strip()
        # dopasuj: Węzeł <node> (Podgraf <pg>): Połączony z: <lista sąsiadów>
        m = re.match(r'Węzeł\s+(\d+)\s+\(Podgraf\s+(\d+)\):\s+Połączony z:\s*(.*)', line)
        if not m:
            continue
        node = int(m.group(1))
        podgraf = int(m.group(2))
        # sparsuj listę sąsiadów, mogą być rozdzielone spacjami
        neighbors = [int(x) for x in m.group(3).split() if x.isdigit()]

        # dodaj wierzchołek z atrybutem 'podgraf'
        G.add_node(node, podgraf=podgraf)
        # dodaj krawędzie (nie dubluje, bo networkx.Graph())
        for nb in neighbors:
            G.add_edge(node, nb)

# 3) Przygotuj paletę kolorów wg liczby unikalnych podgrafów
pod_vals = [data['podgraf'] for _, data in G.nodes(data=True)]
unique_p = sorted(set(pod_vals))
n_p = len(unique_p)

if n_p <= 20:
    palette = plt.cm.tab20.colors[:n_p]
else:
    cmap = plt.cm.get_cmap('gist_ncar', n_p)
    palette = [cmap(i) for i in range(n_p)]

# mapowanie numer podgraf → indeks palety
map_p = {p: i for i, p in enumerate(unique_p)}
# lista kolorów dla każdego wierzchołka w kolejności G.nodes()
colors = [palette[ map_p[G.nodes[v]['podgraf']] ] for v in G.nodes()]

# 4) Rysuj
plt.figure(figsize=(12, 12))
pos = nx.spring_layout(G, seed=42)   # układ siłowy
nx.draw(
    G, pos,
    node_color=colors,
    with_labels=True,
    node_size=200,
    font_size=6,
    edge_color='gray'
)
plt.title(f"Graf: {G.number_of_nodes()} węzłów, {n_p} podgrafów")
plt.axis('off')
plt.show()
