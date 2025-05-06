import networkx as nx
import matplotlib.pyplot as plt
import re

# Inicjalizacja grafu
G = nx.Graph()

# Wczytanie danych z pliku
with open("./testing/Plik2.txt", 'r', encoding='utf-8') as file:
    for line in file:
        line = line.strip()
        match = re.match(r'Węzeł (\d+) \(Podciąg (\d+)\): Połączony z: ([\d\s]+)', line)
        if match:
            node = int(match.group(1))
            podciag = int(match.group(2))
            connections = list(map(int, match.group(3).split()))
            # Dodaj węzeł z atrybutem podciągu
            G.add_node(node, podciag=podciag)
            # Dodaj krawędzie
            for neighbor in connections:
                G.add_edge(node, neighbor)

# Przygotowanie kolorów dla podciągów
podciagi = set(nx.get_node_attributes(G, 'podciag').values())
colors = plt.cm.tab10.colors[:len(podciagi)]  # Kolory z tab10
node_colors = [colors[G.nodes[node]['podciag']] for node in G.nodes()]

# Rysowanie grafu
plt.figure(figsize=(15, 15))
pos = nx.spring_layout(G, seed=42)
nx.draw(G, pos, node_color=node_colors, with_labels=True, node_size=200, font_size=6, edge_color='grey')
plt.title("Graf z kolorowanymi podciągami")
plt.show()