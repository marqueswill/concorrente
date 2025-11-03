#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>  // Para o rand() do putômetro
#include <unistd.h>

#include <domain.c>

// --- VARIÁVEIS GLOBAIS ---

// --- FUNÇÃO PRINCIPAL ---
int main() {
    int i;
    Cozinha cozinha;
    pthread_t threads_pedidos[N_MAX_PEDIDOS];
    PedidoThread infos_pedidos[N_MAX_PEDIDOS];

    srand(time(NULL));

    printf("--- Cozinha Abrindo! ---\n");

    pthread_mutex_init(&cozinha.chef, NULL);

    // Define a quantidade inicial de recursos
    cozinha.bocas_fogao_disponiveis = N_BOCAS_FOGAO;
    cozinha.tabuas_corte_disponiveis = N_TABUAS_CORTE;
    cozinha.geladeira_disponivel = 1;  // Livre
    cozinha.pia_disponivel = 1;        // Livre

    for (i = 0; i < N_MAX_PEDIDOS; i++) {
        cozinha.estado_pedidos[i] = NA_FILA;
        pthread_cond_init(&cozinha.cond_pedido_pode_preparar[i], NULL);
    }

    // 3. CRIAR OS THREADS DE "PEDIDO"
    for (i = 0; i < N_MAX_PEDIDOS; i++) {
        // Prepara a struct de informação para este thread
        infos_pedidos[i].id = i;
        infos_pedidos[i].cozinha = &cozinha;
        infos_pedidos[i].paciencia_maxima = (rand() % 5) + 3;  // Paciência de 3 a 7 segundos

        // TODO : associar receita a cada pedido
    }

    // 4. ESPERAR OS THREADS TERMINAREM (JUNÇÃO)
    for (i = 0; i < N_MAX_PEDIDOS; i++) {
        pthread_join(threads_pedidos[i], NULL);
    }

    // 5. LIMPEZA DOS RECURSOS (Mutexes e Cond_vars)
    printf("--- Cozinha Fechando! Todos os pedidos concluídos. ---\n");
    pthread_mutex_destroy(&cozinha.chef);
    for (i = 0; i < N_MAX_PEDIDOS; i++) {
        pthread_cond_destroy(&cozinha.cond_pedido_pode_preparar[i]);
    }

    return 0;
}