#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 * Verifica KNN por força bruta.
 * Q: nq x D  (float)
 * P: n  x D  (float)
 * R: vetor linearizado de tamanho nq*k, onde R[linha*k + coluna] é índice em [0, n)
 *
 * A função:
 *  - Recalcula o KNN exato (força bruta) para cada linha de Q
 *  - Compara com R (conteúdo e ordem crescente por distância, com tolerância p/ empates)
 *  - Imprime um resumo e até 'max_print' discrepâncias completas
 */
void verificaKNN( float *Q, int nq, float *P, int n, int D, int k, int *R )
{
    const double eps = 1e-7;       /* tolerância para empates numéricos */
    const int max_print = 10;      /* máximo de linhas problemáticas para detalhar */
    int mismatches = 0;
    int order_warnings = 0;
    int out_of_range = 0;

    if (nq <= 0 || n <= 0 || D <= 0 || k <= 0) {
        printf("[verificaKNN] Parâmetros inválidos.\n");
        return;
    }

    /* se k > n, só é possível retornar no máximo n vizinhos */
    int k_eff = (k <= n) ? k : n;
    if (k_eff != k) {
        printf("[verificaKNN] Aviso: k (%d) > n (%d). Verificando com k_eff = %d.\n", k, n, k_eff);
    }

    /* buffers temporários por consulta */
    int   *best_idx  = (int*)   malloc((size_t)k_eff * sizeof(int));
    double*best_dist = (double*)malloc((size_t)k_eff * sizeof(double));
    if (!best_idx || !best_dist) {
        printf("[verificaKNN] Falha de memória.\n");
        free(best_idx); free(best_dist);
        return;
    }

    /* função auxiliar: distância euclidiana ao quadrado entre q_i e p_j */
    auto double dist2(const float* q, const float* p, int D) {
        double s = 0.0;
        for (int d = 0; d < D; ++d) {
            double diff = (double)q[d] - (double)p[d];
            s += diff * diff;
        }
        return s;
    }

    /* ordenação simples (insertion sort) para k pequeno */
    auto void sort_by_dist(int *idx, double *dst, int m) {
        for (int i = 1; i < m; ++i) {
            int    ik = idx[i];
            double dk = dst[i];
            int j = i - 1;
            while (j >= 0 && (dst[j] > dk)) {
                idx[j+1] = idx[j];
                dst[j+1] = dst[j];
                --j;
            }
            idx[j+1] = ik;
            dst[j+1] = dk;
        }
    }

    /* varre todas as consultas */
    int printed = 0;
    for (int qi = 0; qi < nq; ++qi) {

        const float* q = Q + (size_t)qi * D;

        /* mantém os k melhores numa "lista desordenada" com substituição do pior */
        int cur = 0;
        int pos_worst = -1;
        double worst = -1.0;

        for (int pj = 0; pj < n; ++pj) {
            const float* p = P + (size_t)pj * D;
            double d2 = dist2(q, p, D);

            if (cur < k_eff) {
                best_idx[cur]  = pj;
                best_dist[cur] = d2;
                if (pos_worst < 0 || d2 > worst) { pos_worst = cur; worst = d2; }
                ++cur;
            } else {
                if (d2 + eps < worst) {
                    /* substitui o pior */
                    best_idx[pos_worst]  = pj;
                    best_dist[pos_worst] = d2;
                    /* recomputa o pior atual */
                    pos_worst = 0; worst = best_dist[0];
                    for (int t = 1; t < k_eff; ++t) {
                        if (best_dist[t] > worst) { worst = best_dist[t]; pos_worst = t; }
                    }
                }
            }
        }

        /* ordena por distância crescente para comparar à saída esperada */
        sort_by_dist(best_idx, best_dist, k_eff);

        /* checa monotonicidade não-decrescente em R (opcional, ajuda a diagnosticar) */
        int order_ok = 1;
        for (int t = 1; t < k_eff; ++t) {
            double dprev = 0.0, dnow = 0.0;
            int rprev = R[qi * k + (t-1)];
            int rnow  = R[qi * k + t];
            if (rprev < 0 || rprev >= n || rnow < 0 || rnow >= n) continue;
            dprev = dist2(q, P + (size_t)rprev * D, D);
            dnow  = dist2(q, P + (size_t)rnow  * D, D);
            if (dnow + eps < dprev) { order_ok = 0; break; }
        }
        if (!order_ok) order_warnings++;

        /* checa índices fora do intervalo */
        int range_ok = 1;
        for (int t = 0; t < k_eff; ++t) {
            int r = R[qi * k + t];
            if (r < 0 || r >= n) { range_ok = 0; break; }
        }
        if (!range_ok) out_of_range++;

        /* compara conteúdo e ordem (com tolerância para empates):
           - Se as distâncias forem (quase) iguais, permissivo com permutações entre empatados.
           - Caso contrário, exige igualdade posicional. */
        int this_mismatch = 0;

        /* constrói pares (idx,dist) para R a fim de aplicar a mesma regra de empates */
        int   *r_idx  = (int*)malloc((size_t)k_eff * sizeof(int));
        double*r_dist = (double*)malloc((size_t)k_eff * sizeof(double));
        if (!r_idx || !r_dist) {
            printf("[verificaKNN] Falha de memória (linha %d).\n", qi);
            free(r_idx); free(r_dist);
            break;
        }
        for (int t = 0; t < k_eff; ++t) {
            int ridx = R[qi * k + t];
            r_idx[t]  = ridx;
            r_dist[t] = (ridx >= 0 && ridx < n) ? dist2(q, P + (size_t)ridx * D, D) : INFINITY;
        }

        /* verifica igualdade por “blocos de empate”:
           percorre ambos vetores e, para cada faixa de distâncias empatadas,
           checa se os conjuntos de índices coincidem (ordem irrelevante dentro do empate). */
        int i = 0, j = 0;
        while (i < k_eff && j < k_eff) {
            /* define o bloco de empate em best_dist ao redor de best_dist[i] */
            double baseA = best_dist[i];
            int i2 = i + 1;
            while (i2 < k_eff && fabs(best_dist[i2] - baseA) <= eps) i2++;

            /* define o bloco de empate correspondente em r_dist ao redor de r_dist[j] */
            double baseB = r_dist[j];
            int j2 = j + 1;
            while (j2 < k_eff && fabs(r_dist[j2] - baseB) <= eps) j2++;

            /* Se as bases de bloco não estiverem “próximas”, há desordem/erro */
            if (fabs(baseA - baseB) > eps) { this_mismatch = 1; break; }

            /* compara os conjuntos de índices do bloco (ordem irrelevante) */
            int sizeA = i2 - i;
            int sizeB = j2 - j;
            if (sizeA != sizeB) { this_mismatch = 1; break; }

            /* marcação simples O(m^2) (m = tamanho do bloco); suficiente para verificação */
            int *used = (int*)calloc((size_t)sizeB, sizeof(int));
            if (!used) { this_mismatch = 1; break; }

            for (int ua = 0; ua < sizeA && !this_mismatch; ++ua) {
                int idxA = best_idx[i + ua];
                int found = 0;
                for (int ub = 0; ub < sizeB; ++ub) {
                    if (!used[ub] && r_idx[j + ub] == idxA) { used[ub] = 1; found = 1; break; }
                }
                if (!found) this_mismatch = 1;
            }
            free(used);

            if (this_mismatch) break;

            /* avança para o próximo bloco */
            i = i2; j = j2;
        }
        if (i != k_eff || j != k_eff) this_mismatch = 1; /* tamanhos diferentes */

        if (this_mismatch) {
            mismatches++;
            if (printed < max_print) {
                fprintf(stderr, "[verificaKNN] Diferença na linha %d\n", qi);
                fprintf(stderr, "  Esperado (R): ");
                for (int t = 0; t < k_eff; ++t) fprintf(stderr, "%d ", R[qi * k + t]);
                fprintf(stderr, "\n  BruteForce : ");
                for (int t = 0; t < k_eff; ++t) fprintf(stderr, "%d ", best_idx[t]);
                fprintf(stderr, "\n");
                printed++;
            }
        }

        free(r_idx);
        free(r_dist);
    }

    free(best_idx);
    free(best_dist);

    printf("------------ VERIFICA KNN ---------------\n");
    printf("Consultas verificadas: %d | k_eff=%d\n", nq, (k <= n) ? k : n);
    if (out_of_range)  printf("Indices fora do intervalo em R: %d linhas\n", out_of_range);
    if (order_warnings)printf("Aviso: ordem estritamente crescente por distancia violada em %d linhas (tolerancia eps=%g)\n",
                              order_warnings, eps);
    if (mismatches == 0) {
        printf("Resultado OK: todos os %d vetores de vizinhos batem com o brute force (com tolerancia).\n", nq);
    } else {
        printf("Resultado DIFERE em %d/%d consultas. Veja acima as primeiras discrepancias.\n", mismatches, nq);
    }
}
