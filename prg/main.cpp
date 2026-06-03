// microprime_M30_doppio_motore.cpp
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdint>

#ifdef _OPENMP
#include <omp.h>
#endif

using u64 = unsigned long long;

inline int count_bits(uint8_t x) {
    return __builtin_popcount(x);
}

std::vector<u64> calcola_divisori_noti(u64 limite_p) {
    std::vector<u64> divisori;
    divisori.reserve(limite_p / 10);
    std::vector<uint8_t> sieve_base(limite_p + 1, 1);
    sieve_base[0] = 0; sieve_base[1] = 0;
    for (u64 p = 2; p <= limite_p; p++) {
        if (sieve_base[p]) {
            divisori.push_back(p);
            u64 a = p * p;
            while (a <= limite_p) { sieve_base[a] = 0; a += p; }
        }
    }
    std::vector<u64> divisori_utili;
    for (u64 p : divisori) {
        if (p > 5) {
            int mod30 = p % 30;
            if (mod30 == 1 || mod30 == 7 || mod30 == 11 || mod30 == 13 ||
                mod30 == 17 || mod30 == 19 || mod30 == 23 || mod30 == 29) {
                divisori_utili.push_back(p);
            }
        }
    }
    return divisori_utili;
}

// =========================================================================
// MOTORE 1: Setaccio Singolo Veloce (Il tuo codice originale intatto)
// =========================================================================
void esegui_setaccio_singolo(u64 N) {
    std::cout << "======================================================" << std::endl;
    std::cout << "  MICROPRIME FAST (Modalita' Singola in RAM)" << std::endl;
    std::cout << "  Finestra limite N = " << N << std::endl;
    std::cout << "======================================================" << std::endl;

    auto start_total = std::chrono::high_resolution_clock::now();
    u64 max_blocchi = (N / 30) + 1;
    u64 limite_p = (u64)std::sqrt((double)N);

    std::vector<u64> divisori_noti = calcola_divisori_noti(limite_p);

    std::vector<uint8_t> sieve_0(max_blocchi, 0x03);
    std::vector<uint8_t> sieve_10(max_blocchi, 0x0F);
    std::vector<uint8_t> sieve_20(max_blocchi, 0x03);
    sieve_0[0] &= ~(1 << 0);

    const u64 SEG_SIZE = 131072;

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif
    for (long long s_start_ll = 0; s_start_ll < (long long)max_blocchi; s_start_ll += (long long)SEG_SIZE) {
        u64 s_start = (u64)s_start_ll;
        u64 s_end = std::min(s_start + SEG_SIZE, max_blocchi);
        u64 seg_len = s_end - s_start;

        alignas(64) uint8_t seg0[SEG_SIZE];  std::memcpy(seg0,  sieve_0.data() + s_start, seg_len);
        alignas(64) uint8_t seg10[SEG_SIZE]; std::memcpy(seg10, sieve_10.data() + s_start, seg_len);
        alignas(64) uint8_t seg20[SEG_SIZE]; std::memcpy(seg20, sieve_20.data() + s_start, seg_len);

        for (u64 p : divisori_noti) {
            u64 a_base = p * p;
            u64 step_val = p * 2;
            u64 entry[8] = { u64(-1), u64(-1), u64(-1), u64(-1), u64(-1), u64(-1), u64(-1), u64(-1) };
            u64 a = a_base;

            for(int i=0; i<15; ++i) {
                if (a > N) break;
                int mod30 = a % 30; u64 blocco = a / 30;
                if      (mod30 == 1)  entry[0] = blocco; else if (mod30 == 7)  entry[1] = blocco;
                else if (mod30 == 11) entry[2] = blocco; else if (mod30 == 13) entry[3] = blocco;
                else if (mod30 == 17) entry[4] = blocco; else if (mod30 == 19) entry[5] = blocco;
                else if (mod30 == 23) entry[6] = blocco; else if (mod30 == 29) entry[7] = blocco;
                a += step_val;
            }

            for(int i=0; i<8; ++i) {
                if (entry[i] != u64(-1)) {
                    u64 blocco = entry[i];
                    if (blocco < s_start) { blocco += ((s_start - blocco + p - 1) / p) * p; }

                    if (i == 0)      while (blocco < s_end) { seg0[blocco - s_start]  &= ~(1 << 0); blocco += p; }
                    else if (i == 1) while (blocco < s_end) { seg0[blocco - s_start]  &= ~(1 << 1); blocco += p; }
                    else if (i == 2) while (blocco < s_end) { seg10[blocco - s_start] &= ~(1 << 0); blocco += p; }
                    else if (i == 3) while (blocco < s_end) { seg10[blocco - s_start] &= ~(1 << 1); blocco += p; }
                    else if (i == 4) while (blocco < s_end) { seg10[blocco - s_start] &= ~(1 << 2); blocco += p; }
                    else if (i == 5) while (blocco < s_end) { seg10[blocco - s_start] &= ~(1 << 3); blocco += p; }
                    else if (i == 6) while (blocco < s_end) { seg20[blocco - s_start] &= ~(1 << 0); blocco += p; }
                    else if (i == 7) while (blocco < s_end) { seg20[blocco - s_start] &= ~(1 << 1); blocco += p; }
                }
            }
        }
        std::memcpy(sieve_0.data() + s_start,  seg0,  seg_len);
        std::memcpy(sieve_10.data() + s_start, seg10, seg_len);
        std::memcpy(sieve_20.data() + s_start, seg20, seg_len);
    }

    u64 conteggio = 3;
    u64 ultimo_blocco = N / 30;
    for (u64 blocco = 0; blocco < ultimo_blocco; blocco++) {
        conteggio += count_bits(sieve_0[blocco]) + count_bits(sieve_10[blocco]) + count_bits(sieve_20[blocco]);
    }

    uint8_t l0 = sieve_0[ultimo_blocco], l10 = sieve_10[ultimo_blocco], l20 = sieve_20[ultimo_blocco];
    if ((l0 & (1<<0)) && (ultimo_blocco*30+1 <= N) && (ultimo_blocco*30+1 > 1)) conteggio++;
    if ((l0 & (1<<1)) && (ultimo_blocco*30+7 <= N)) conteggio++;
    if ((l10 & (1<<0)) && (ultimo_blocco*30+11 <= N)) conteggio++;
    if ((l10 & (1<<1)) && (ultimo_blocco*30+13 <= N)) conteggio++;
    if ((l10 & (1<<2)) && (ultimo_blocco*30+17 <= N)) conteggio++;
    if ((l10 & (1<<3)) && (ultimo_blocco*30+19 <= N)) conteggio++;
    if ((l20 & (1<<0)) && (ultimo_blocco*30+23 <= N)) conteggio++;
    if ((l20 & (1<<1)) && (ultimo_blocco*30+29 <= N)) conteggio++;

    auto end_total = std::chrono::high_resolution_clock::now();
    double tempo_totale = std::chrono::duration<double>(end_total - start_total).count();
    std::cout << "Primi totali rilevati: " << conteggio << " in " << tempo_totale << " secondi.\n";
}

// =========================================================================
// MOTORE 2: Setaccio Segmentato Continuo (Ripetizione a zero memoria)
// =========================================================================
void esegui_setaccio_ripetuto(u64 dimensione_segmento, int ripetizioni) {
    u64 MAX_N = dimensione_segmento * (u64)ripetizioni;

    std::cout << "======================================================" << std::endl;
    std::cout << "  MICROPRIME FAST (Modalita' Ripetuta / Segmentata Sicura)" << std::endl;
    std::cout << "  Esplorazione fino a N = " << MAX_N << std::endl;
    std::cout << "  Cicli: " << ripetizioni << " da " << dimensione_segmento << " numeri." << std::endl;
    std::cout << "======================================================" << std::endl;

    auto start_total = std::chrono::high_resolution_clock::now();

    // I divisori coprono la radice del target FINALE
    u64 limite_p = (u64)std::sqrt((double)MAX_N);
    std::vector<u64> divisori_noti = calcola_divisori_noti(limite_p);

    u64 total_blocchi = (MAX_N / 30) + 1;
    u64 blocchi_per_giro = (total_blocchi + ripetizioni - 1) / ripetizioni;
    u64 conteggio_globale = 3; // 2, 3, 5

    std::vector<std::vector<u64>> entry_base(divisori_noti.size(), std::vector<u64>(8, u64(-1)));

    #ifdef _OPENMP
    #pragma omp parallel for schedule(dynamic, 1)
    #endif
    for (long long d = 0; d < (long long)divisori_noti.size(); d++) {
        u64 p = divisori_noti[d];
        u64 a = p * p;
        u64 step_val = p * 2;

        for(int i=0; i<15; ++i) {
            int mod30 = a % 30; u64 blocco = a / 30;
            if      (mod30 == 1  && entry_base[d][0] == u64(-1)) entry_base[d][0] = blocco;
            else if (mod30 == 7  && entry_base[d][1] == u64(-1)) entry_base[d][1] = blocco;
            else if (mod30 == 11 && entry_base[d][2] == u64(-1)) entry_base[d][2] = blocco;
            else if (mod30 == 13 && entry_base[d][3] == u64(-1)) entry_base[d][3] = blocco;
            else if (mod30 == 17 && entry_base[d][4] == u64(-1)) entry_base[d][4] = blocco;
            else if (mod30 == 19 && entry_base[d][5] == u64(-1)) entry_base[d][5] = blocco;
            else if (mod30 == 23 && entry_base[d][6] == u64(-1)) entry_base[d][6] = blocco;
            else if (mod30 == 29 && entry_base[d][7] == u64(-1)) entry_base[d][7] = blocco;
            a += step_val;
        }
    }

    const u64 SEG_SIZE = 131072;

    // Ciclo Principale delle Ripetizioni
    for (int giro = 0; giro < ripetizioni; giro++) {
        u64 N_start = dimensione_segmento * (u64)giro;
        u64 N_giro  = dimensione_segmento * (u64)(giro + 1);
        u64 global_s_start = N_start / 30;
        u64 global_s_end   = std::min(N_giro / 30 + 1, total_blocchi);
        if (global_s_start >= total_blocchi) break;

        u64 conteggio_giro = 0;

        // Parallelizziamo sui CHUNK di memoria per evitare Data Race
        #ifdef _OPENMP
        #pragma omp parallel for schedule(dynamic, 1) reduction(+:conteggio_giro)
        #endif
        for (long long s_start_ll = global_s_start; s_start_ll < (long long)global_s_end; s_start_ll += (long long)SEG_SIZE) {
            u64 s_start = (u64)s_start_ll;
            u64 s_end = std::min(s_start + SEG_SIZE, global_s_end);
            u64 seg_len = s_end - s_start;

            alignas(64) uint8_t seg0[SEG_SIZE];  std::fill(seg0, seg0 + seg_len, 0x03);
            alignas(64) uint8_t seg10[SEG_SIZE]; std::fill(seg10, seg10 + seg_len, 0x0F);
            alignas(64) uint8_t seg20[SEG_SIZE]; std::fill(seg20, seg20 + seg_len, 0x03);

            if (s_start == 0) seg0[0] &= ~(1 << 0); // Il numero 1 non e' primo

            for (size_t d = 0; d < divisori_noti.size(); d++) {
                u64 p = divisori_noti[d];
                for(int i=0; i<8; ++i) {
                    u64 blocco = entry_base[d][i];
                    if (blocco != u64(-1)) {
                        if (blocco < s_start) {
                            blocco += ((s_start - blocco + p - 1) / p) * p;
                        }

                        if (i == 0)      while (blocco < s_end) { seg0[blocco - s_start]  &= ~(1 << 0); blocco += p; }
                        else if (i == 1) while (blocco < s_end) { seg0[blocco - s_start]  &= ~(1 << 1); blocco += p; }
                        else if (i == 2) while (blocco < s_end) { seg10[blocco - s_start] &= ~(1 << 0); blocco += p; }
                        else if (i == 3) while (blocco < s_end) { seg10[blocco - s_start] &= ~(1 << 1); blocco += p; }
                        else if (i == 4) while (blocco < s_end) { seg10[blocco - s_start] &= ~(1 << 2); blocco += p; }
                        else if (i == 5) while (blocco < s_end) { seg10[blocco - s_start] &= ~(1 << 3); blocco += p; }
                        else if (i == 6) while (blocco < s_end) { seg20[blocco - s_start] &= ~(1 << 0); blocco += p; }
                        else if (i == 7) while (blocco < s_end) { seg20[blocco - s_start] &= ~(1 << 1); blocco += p; }
                    }
                }
            }

            // Conteggio al volo con controllo esatto per l'ultimo blocco di ogni giro
            u64 ultimo_blocco_giro = N_giro / 30;
            u64 local_count = 0;
            for(u64 b = 0; b < seg_len; b++) {
                u64 blocco_assoluto = s_start + b;
                if (blocco_assoluto == ultimo_blocco_giro) {
                    uint8_t l0 = seg0[b], l10 = seg10[b], l20 = seg20[b];
                    if ((l0 & (1<<0)) && (blocco_assoluto*30+1 <= N_giro) && (blocco_assoluto*30+1 > 1)) local_count++;
                    if ((l0 & (1<<1)) && (blocco_assoluto*30+7 <= N_giro)) local_count++;
                    if ((l10 & (1<<0)) && (blocco_assoluto*30+11 <= N_giro)) local_count++;
                    if ((l10 & (1<<1)) && (blocco_assoluto*30+13 <= N_giro)) local_count++;
                    if ((l10 & (1<<2)) && (blocco_assoluto*30+17 <= N_giro)) local_count++;
                    if ((l10 & (1<<3)) && (blocco_assoluto*30+19 <= N_giro)) local_count++;
                    if ((l20 & (1<<0)) && (blocco_assoluto*30+23 <= N_giro)) local_count++;
                    if ((l20 & (1<<1)) && (blocco_assoluto*30+29 <= N_giro)) local_count++;
                } else {
                    local_count += count_bits(seg0[b]) + count_bits(seg10[b]) + count_bits(seg20[b]);
                }
            }
            conteggio_giro += local_count;
        }

        conteggio_globale += conteggio_giro;

        u64 limite_giro = dimensione_segmento * (u64)(giro + 1);
        std::cout << "Segmento " << (giro + 1) << " (0 - " << limite_giro << "): "
                  << conteggio_globale << " primi" << std::endl;
    }

    auto end_total = std::chrono::high_resolution_clock::now();
    double tempo_totale = std::chrono::duration<double>(end_total - start_total).count();

    std::cout << "Primi totali rilevati: " << conteggio_globale << std::endl;
    std::cout << "Tempo globale di elaborazione: " << tempo_totale << " secondi" << std::endl;
    std::cout << "Memoria RAM utilizzata per i vettori: Pochissima (Cache L1/L2 Perfetta)!" << std::endl;
}

// =========================================================================
// MAIN ROUTER (Gestisce gli argomenti e instrada)
// =========================================================================
int main(int argc, char* argv[]) {
    if (argc == 1) {
        esegui_setaccio_singolo(100'000'000ULL);
    }
    else if (argc == 2) {
        u64 N = std::stoull(argv[1]);
        esegui_setaccio_singolo(N);
    }
    else if (argc == 3) {
        u64 segmento = std::stoull(argv[1]);
        int ripetizioni = std::stoi(argv[2]);
        esegui_setaccio_ripetuto(segmento, ripetizioni);
    }
    return 0;
}
