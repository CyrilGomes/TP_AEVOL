//
// Created by arrouan on 01/10/18.
//

#include "Dna.h"
#include "omp.h"

#include <cassert>

Dna::Dna(int length, Threefry::Gen &&rng) : seq_(length) {
    // Generate a random genome
    for (int32_t i = 0; i < length; i++) {
        seq_[i] = '0' + rng.random(NB_BASE);
    }
}

int Dna::length() const {
    return seq_.size();
}

void Dna::save(gzFile backup_file) {
    int dna_length = length();
    gzwrite(backup_file, &dna_length, sizeof(dna_length));
    gzwrite(backup_file, seq_.data(), dna_length * sizeof(seq_[0]));
}

void Dna::load(gzFile backup_file) {
    int dna_length;
    gzread(backup_file, &dna_length, sizeof(dna_length));

    char tmp_seq[dna_length];
    gzread(backup_file, tmp_seq, dna_length * sizeof(tmp_seq[0]));

    seq_ = std::vector<char>(tmp_seq, tmp_seq + dna_length);
}

void Dna::set(int pos, char c) {
    seq_[pos] = c;
}

/**
 * Remove the DNA inbetween pos_1 and pos_2
 *
 * @param pos_1
 * @param pos_2
 */
void Dna::remove(int pos_1, int pos_2) {
    assert(pos_1 >= 0 && pos_2 >= pos_1 && pos_2 <= seq_.size());
    seq_.erase(seq_.begin() + pos_1, seq_.begin() + pos_2);
}

/**
 * Insert a sequence of a given length at a given position into the DNA of the Organism
 *
 * @param pos : where to insert the sequence
 * @param seq : the sequence itself
 * @param seq_length : the size of the sequence
 */
void Dna::insert(int pos, std::vector<char> seq) {
// Insert sequence 'seq' at position 'pos'
    assert(pos >= 0 && pos < seq_.size());

    seq_.insert(seq_.begin() + pos, seq.begin(), seq.end());
}

/**
 * Insert a sequence of a given length at a given position into the DNA of the Organism
 *
 * @param pos : where to insert the sequence
 * @param seq : the sequence itself
 * @param seq_length : the size of the sequence
 */
void Dna::insert(int pos, Dna *seq) {
// Insert sequence 'seq' at position 'pos'
    assert(pos >= 0 && pos < seq_.size());

    seq_.insert(seq_.begin() + pos, seq->seq_.begin(), seq->seq_.end());
}

void Dna::do_switch(int pos) {
    if (seq_[pos] == '0') seq_[pos] = '1';
    else seq_[pos] = '0';
}

void Dna::do_duplication(int pos_1, int pos_2, int pos_3) {
    // Duplicate segment [pos_1; pos_2[ and insert the duplicate before pos_3
    char *duplicate_segment = NULL;

    int32_t seg_length;

    if (pos_1 < pos_2) {
        //
        //       pos_1         pos_2                   -> 0-
        //         |             |                   -       -
        // 0--------------------------------->      -         -
        //         ===============                  -         - pos_1
        //           tmp (copy)                      -       -
        //                                             -----      |
        //                                             pos_2    <-'
        //
        std::vector<char> seq_dupl =
                std::vector<char>(seq_.begin() + pos_1, seq_.begin() + pos_2);

        insert(pos_3, seq_dupl);
    } else { // if (pos_1 >= pos_2)
        // The segment to duplicate includes the origin of replication.
        // The copying process will be done in two steps.
        //
        //                                            ,->
        //    pos_2                 pos_1            |      -> 0-
        //      |                     |                   -       - pos_2
        // 0--------------------------------->     pos_1 -         -
        // ======                     =======            -         -
        //  tmp2                        tmp1              -       -
        //                                                  -----
        //
        //
        std::vector<char>
                seq_dupl = std::vector<char>(seq_.begin() + pos_1, seq_.end());
        seq_dupl.insert(seq_dupl.end(), seq_.begin(), seq_.begin() + pos_2);

        insert(pos_3, seq_dupl);
    }
}

void Dna::extend_seq_to_prom_size() {
    // Extend the sequence to avoid modulo, create a copy, and extend it and then put it in seq_ext
    std::vector<char> seq_cpy = seq_;
    seq_cpy.insert(seq_cpy.end(), seq_.begin(), seq_.begin() + PROM_SIZE);
    seq_ext = seq_cpy.data();
}

int Dna::promoter_at(int pos) {
    int prom_dist[PROM_SIZE];
    int dist_lead = 0;

    #pragma omp simd reduction(+:dist_lead)
    for (int motif_id = 0; motif_id < PROM_SIZE; motif_id++) {

        // Searching for the promoter
        prom_dist[motif_id] = PROM_SEQ[motif_id] != seq_ext[pos + motif_id];
        dist_lead += prom_dist[motif_id];


    }
    
    return dist_lead;
}

// Given a, b, c, d boolean variable and X random boolean variable,
// a terminator look like : a b c d X X !d !c !b !a
int Dna::terminator_at(int pos) {
    int term_dist[TERM_STEM_SIZE];

    int size=length();
    int dist_term_lead=0;
    #pragma omp simd reduction(+:dist_term_lead)
    for (int motif_id = 0; motif_id < TERM_STEM_SIZE; motif_id++) {
        int right = pos + motif_id;
        int left = pos + (TERM_SIZE - 1) - motif_id;

        // loop back the dna inf needed
        

        // Search for the terminators
        term_dist[motif_id] = seq_ext[right] != seq_ext[left];
        dist_term_lead+=term_dist[motif_id];
    }

    return dist_term_lead;
}

bool Dna::shine_dal_start(int pos) {
    bool start = false;
    int t_pos, k_t;

    for (int k = 0; k < SHINE_DAL_SIZE + CODON_SIZE; k++) {
        k_t = k >= SHINE_DAL_SIZE ? k + SD_START_SPACER : k;
        t_pos = pos + k_t;
        if (t_pos >= seq_.size())
            t_pos -= seq_.size();

        if (seq_[t_pos] == SHINE_DAL_SEQ[k_t]) {
            start = true;
        } else {
            start = false;
            break;
        }
    }

    return start;
}

bool Dna::protein_stop(int pos) {
    bool is_protein;
    int t_k;

    for (int k = 0; k < CODON_SIZE; k++) {
        t_k = pos + k;
        if (t_k >= seq_.size())
            t_k -= seq_.size();

        if (seq_[t_k] == PROTEIN_END[k]) {
            is_protein = true;
        } else {
            is_protein = false;
            break;
        }
    }

    return is_protein;
}

int Dna::codon_at(int pos) {
    int value = 0;

    int t_pos;

    for (int i = 0; i < CODON_SIZE; i++) {
        t_pos = pos + i;
        if (t_pos >= seq_.size())
            t_pos -= seq_.size();
        if (seq_[t_pos] == '1')
            value += 1 << (CODON_SIZE - i - 1);
    }

    return value;
}