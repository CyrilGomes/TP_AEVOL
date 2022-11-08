## simulation_0 : 
version originale
## simulation_1 à simulation_4 : 
Fichier : Dna.cpp
```
int Dna::promoter_at(int pos) {
    int prom_dist[PROM_SIZE];
    int dist_lead = 0;
    for (int motif_id = 0; motif_id < PROM_SIZE; motif_id++) {
        int search_pos = pos + motif_id;
        if (search_pos >= seq_.size())
            search_pos -= seq_.size();
        // Searching for the promoter
        prom_dist[motif_id] =
                PROM_SEQ[motif_id] == seq_[search_pos] ? 0 : 1;
        dist_lead += prom_dist[motif_id];
    }
    return dist_lead;
}
```
