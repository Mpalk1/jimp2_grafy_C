//mozna tego uzyc jak ogarne jak naprawic

// static void balance_partitions(t_graph *graph, int num_parts, int margin,
//     size_t *sizes) { // do wyrzucenia
//         calculate_partition_sizes(graph, num_parts, sizes);
//         size_t ideal = graph->nodes_num / num_parts;
//         int allowed_diff = ideal * margin / 100;
    
//         bool balanced = false;
//         while (!balanced) {
//             balanced = true;
    
//             for (int i = 0; i < num_parts; i++) {
//                 if (sizes[i] > ideal + allowed_diff) {
//                     for (int j = 0; j < num_parts; j++) {
//                         if (sizes[j] < ideal - allowed_diff) {
//                             for (size_t n = 0; n < graph->nodes_num; n++) {
//                                 if (graph->nodes[n].partition != i)
//                                     continue;
    
//                                 bool has_connection_to_j = false;
//                                 for (size_t c = 0; c <
//                                 graph->nodes[n].connections_num; c++) {
//                                     if (graph->nodes[n].connections[c]->partition
//                                     == j) {
//                                         has_connection_to_j = true;
//                                         break;
//                                     }
//                                 }
    
//                                 if (has_connection_to_j) {
//                                     graph->nodes[n].partition = j;
//                                     sizes[i]--;
//                                     sizes[j]++;
//                                     balanced = false;
//                                     break;
//                                 }
//                             }
//                             if (!balanced) break;
//                         }
//                     }
//                     if (!balanced) break;
//                 }
//             }
//         }
//     }