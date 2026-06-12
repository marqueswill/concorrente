#define _XOPEN_SOURCE 600 /* Or higher */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 3  // Altura da árvore
#define LEAFS (1 << N)
#define NODES ((1 << (N + 1)) - 1)

// Threads são organizadas em uma árvore binária
// Sincronização feita bottom-up, das folhas até a raiz, duas threads por vez.
// Os nós representam os estados das threads: esperando filhos, esperando irmão e sincronizado
// Cada nó tem uma barreira com três permissões para sua thread e a thread dos filhos.
// A thread de cada nó filho se encontra nas barreiras do nó pai.
// Quando se econtrarem, o pai calcula a média das velocidades, atualizam o nó pai e fazem brodcast do valor para baixo
typedef struct TreeNode {
    int id;
    // int thread_state;  // Necessário?
    // int node_height;   // Necessário?
    int is_root;
    int is_leaf;
    struct TreeNode* parent;
    struct TreeNode* left;      // Ponteiro para o filho esquerdo
    struct TreeNode* right;     // Ponteiro para o filho direito
    pthread_barrier_t barrier;  // 3 permissões, thread do nó só avança após as threads filhas chegarem e a média for calculada
} TreeNode;

struct AudioTrack {
    int bpm;
    pthread_mutex_t lock;  // Lock para atualizar o bpm
    pthread_cond_t cond;   // Evita busy waiting
};

pthread_t audio_threads[NODES];  // Threads que reproduzem o aúdio
pthread_t sync_threads[NODES];   // Threads que interagem com a árvore
int thread_ids[NODES];

void* cantor(void* arg);
void* audio(void* arg);

TreeNode* build_tree() {
    // TODO: Construir a árvore em memória alocando os nós dinamicamente
    // TODO: Inicializar ponteiros left, right e parent da árvore
    // TODO: Inicializar as barreiras de cada nó (pthread_barrier_init)

    TreeNode* node_addr[NODES];

    for (int i = 0; i < NODES; i++) {
        TreeNode* node_ptr = malloc(sizeof(TreeNode));
        node_ptr->id = i;
        node_ptr->is_root = 0;
        node_ptr->is_leaf = 0;
        node_ptr->parent = NULL;
        node_ptr->left = NULL;
        node_ptr->right = NULL;

        node_addr[i] = node_ptr;
    }

    int first_leaf_ind = LEAFS - 1;
    for (int i = 0; i < NODES; i++) {
        TreeNode* node = node_addr[i];

        int parent_idx = (i - 1) / 2;
        int left_idx = 2 * i + 1;
        int right_idx = 2 * i + 2;

        if (i > 0) {
            node->parent = node_addr[parent_idx];
        }

        if (left_idx < NODES) {
            node->left = node_addr[left_idx];
        }

        if (right_idx < NODES) {
            node->right = node_addr[right_idx];
        }

        node->is_root = i == 0;
        node->is_leaf = (left_idx >= NODES) || (right_idx >= NODES);

        pthread_barrier_init(&(node->barrier), NULL, (right_idx >= NODES) ? 3 : 1);
    }

    return node_addr[0];
}

int main() {
    TreeNode* tree = build_tree();
    // TODO: Inicializar mutexes e variáveis de condição (pthread_cond_init) das faixas de áudio

    for (int i = 0; i < NODES; i++) {
        thread_ids[i] = i;
        pthread_create(&sync_threads[i], NULL, cantor, &thread_ids[i]);
        pthread_create(&audio_threads[i], NULL, audio, &thread_ids[i]);
    }

    for (int i = 0; i < NODES; i++) {
        pthread_join(sync_threads[i], NULL);
    }

    for (int i = 0; i < NODES; i++) {
        pthread_join(audio_threads[i], NULL);
    }

    // TODO: Destruir barreiras, mutexes e variáveis de condição (destroy)
    // TODO: Liberar a memória da árvore com free() percorrendo os nós

    return 0;
}

void* cantor(void* arg) {
    int id = *((int*)arg);

    while (1) {
        // Se a thread é folha, tento sincronizar com a irmã acessando o nó do pai

        // Se a thread não é folha:
        // -> Está esperando os filhos, faço nada até que os filhos chegem: acesso a barreira do nó atual
        // -> Quando o filhos chegarem: eles acessaram a barreira pela esquerda e pela direita
        // --> Eu calculo a média das velocidades
        // --> Eu propago essa nova média para a subárvore esquerda e direita
        // --> Se o nó atual não é a raiz
        // ---> Eu tento sincronizar com a thread irmã a partir do nó do pai: acesso a barreira do pai

        // TODO: Lógica de navegação bottom-up usando o ponteiro 'parent' do nó
        /*
                int res = pthread_barrier_wait(&no_atual->barrier);
                if (res == PTHREAD_BARRIER_SERIAL_THREAD) {
                    // Apenas UMA das 3 threads entra neste bloco.
                    // Executar o cálculo de média aqui.
                }
        */

        // TODO: Propagar a média
    }

    return NULL;
}

// Ineficiente?
void* audio(void* arg) {
    int id = *((int*)arg);

    // TODO: Iniciar reprodução

    while (1) {
        // TODO: Ler ler o bpm de tracks[id]
        // TODO: Reiniciar reprodução com nova velocidade se o bpm mudar

        /*
        pthread_mutex_lock(&track->lock);
        pthread_cond_wait(&track->cond, &track->lock);

        // Variável de condição acordou a thread e retomou o lock automaticamente.
        // Ler o novo bpm e reiniciar reprodução.

        pthread_mutex_unlock(&track->lock);
        */
    }

    return NULL;
}