import networkx as nx
import matplotlib.pyplot as plt
import re
import numpy as np

# plik do wizualnego debugowania algorytmu
# nie jest czescia programu

# Initialize the graph
G = nx.Graph()

# Read data from file
with open("./testing/Plik2.txt", 'r', encoding='utf-8') as file:
    for line in file:
        line = line.strip()
        match = re.match(r'Węzeł (\d+) \(Podciąg (\d+)\): Połączony z: ([\d\s]+)', line)
        if match:
            node = int(match.group(1))
            podciag = int(match.group(2))
            connections = list(map(int, match.group(3).split()))
            # Add node with podciag attribute
            G.add_node(node, podciag=podciag)
            # Add edges
            for neighbor in connections:
                G.add_edge(node, neighbor)

# Prepare colors for podciags
podciags = list(nx.get_node_attributes(G, 'podciag').values())
unique_podciags = sorted(set(podciags))
num_podciags = len(unique_podciags)

# Create a colormap with 100 distinct colors
if num_podciags <= 20:
    # Use tab20 if we have few enough categories
    colors = plt.cm.tab20.colors[:num_podciags]
else:
    # Create a custom colormap with 100 distinct colors
    colormap = plt.cm.get_cmap('gist_ncar', 100)  # Can produce up to 256 distinct colors
    colors = [colormap(i) for i in np.linspace(0, 1, num_podciags)]

# Create mapping from podciag to color index
podciag_to_index = {podciag: idx for idx, podciag in enumerate(unique_podciags)}

# Assign colors to nodes
node_colors = [colors[podciag_to_index[G.nodes[node]['podciag']]] for node in G.nodes()]

# Draw the graph
plt.figure(figsize=(15, 15))
pos = nx.spring_layout(G, seed=42)
nx.draw(G, pos, node_color=node_colors, with_labels=True, 
        node_size=200, font_size=6, edge_color='grey')
plt.title(f"Graf z kolorowanymi podciągami (liczba podciągów: {num_podciags})")
plt.show()