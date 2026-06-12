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
typedef struct AudioTrack {
    int bpm;
    pthread_mutex_t lock;  // Lock para atualizar o bpm
    pthread_cond_t cond;   // Evita busy waiting
} AudioTrack;

pthread_t audio_threads[NODES];  // Threads que reproduzem o aúdio
pthread_t sync_threads[NODES];   // Threads que interagem com a árvore

int thread_ids[NODES];

AudioTrack* track_list[NODES];
// TreeNode* tree;
TreeNode* tree[NODES];

void* cantor(void* arg);
void* audio(void* arg);

void build_tree() {
    for (int i = 0; i < NODES; i++) {
        TreeNode* node_ptr = malloc(sizeof(TreeNode));
        node_ptr->id = i;
        node_ptr->is_root = 0;
        node_ptr->is_leaf = 0;
        node_ptr->parent = NULL;
        node_ptr->left = NULL;
        node_ptr->right = NULL;

        tree[i] = node_ptr;
    }

    int first_leaf_ind = LEAFS - 1;
    for (int i = 0; i < NODES; i++) {
        TreeNode* node = tree[i];

        int parent_idx = (i - 1) / 2;
        int left_idx = 2 * i + 1;
        int right_idx = 2 * i + 2;

        if (i > 0) {
            node->parent = tree[parent_idx];
        }

        if (left_idx < NODES) {
            node->left = tree[left_idx];
        }

        if (right_idx < NODES) {
            node->right = tree[right_idx];
        }

        node->is_root = i == 0;
        node->is_leaf = (left_idx >= NODES) || (right_idx >= NODES);

        pthread_barrier_init(&(node->barrier), NULL, (right_idx >= NODES) ? 1 : 3);
    }
}

void init_track_data() {
    for (int i = 0; i < NODES; i++) {
        AudioTrack* track_ptr = malloc(sizeof(AudioTrack));
        track_ptr->bpm = (rand() % 200) + 1;  // random de 1 a 200

        pthread_mutex_init(&(track_ptr->lock), NULL);
        pthread_cond_init(&(track_ptr->cond), NULL);

        track_list[i] = track_ptr;
    }
}

// Só funciona em heaps perfeitos
void broadcast_bpm(int track_id, int new_bpm) {
    int left_idx = 2 * track_id + 1;
    int right_idx = 2 * track_id + 2;

    if ((left_idx >= NODES) || (right_idx >= NODES)) {
        return;
    }

    AudioTrack* track = track_list[track_id];

    pthread_mutex_lock(&track->lock);
    track->bpm = new_bpm;
    pthread_cond_signal(&track->cond);  // aviso pra thread de audio atualizar
    pthread_mutex_unlock(&track->lock);

    broadcast_bpm(left_idx, new_bpm);
    broadcast_bpm(right_idx, new_bpm);
}

void play_track(AudioTrack* track) {
    // TODO: implementar essa bomba
    return;
}

int main() {
    build_tree();
    init_track_data();

    // Criar threads
    for (int i = 0; i < NODES; i++) {
        thread_ids[i] = i;
        pthread_create(&sync_threads[i], NULL, cantor, &thread_ids[i]);
        pthread_create(&audio_threads[i], NULL, audio, &thread_ids[i]);
    }

    // Juntar threads
    for (int i = 0; i < NODES; i++) {
        pthread_join(sync_threads[i], NULL);
    }

    for (int i = 0; i < NODES; i++) {
        pthread_join(audio_threads[i], NULL);
    }

    // Liberar memória
    for (int i = 0; i < NODES; i++) {
        free(tree[i]);
    }

    for (int i = 0; i < NODES; i++) {
        pthread_mutex_destroy(&(track_list[i]->lock));
        pthread_cond_destroy(&(track_list[i]->cond));
        free(track_list[i]);
    }

    return 0;
}

void* cantor(void* arg) {
    int id = *((int*)arg);
    TreeNode* node = tree[id];
    AudioTrack* track = track_list[id];
    AudioTrack* left_track = (node->is_leaf) ? NULL : track_list[node->left->id];
    AudioTrack* right_track = (node->is_leaf) ? NULL : track_list[node->right->id];

    while (1) {
        if (node->is_leaf) {                               // Se a thread é folha, tento sincronizar com a irmã acessando o nó do pai
            pthread_barrier_wait(&node->parent->barrier);  // Avanço na árvore tentando liberar a barreira no pai
            break;                                         // Sincronizei, em teoria essa thread não faz mais nada
        } else {                                           //  Se a thread não é folha:
            // Está esperando os filhos, faço nada até que os filhos chegem: acesso a barreira do nó atual
            // Quando o filhos chegarem (eles acessaram a barreira pela esquerda e pela direita)
            int res = pthread_barrier_wait(&node->barrier);
            if (res == PTHREAD_BARRIER_SERIAL_THREAD) {  // Apenas UMA das 3 threads entra neste bloco.
                //  Eu calculo a média das velocidades
                int newBPM = (left_track->bpm + right_track->bpm) / 2;

                //  Eu propago essa nova média para a subárvore esquerda e direita
                broadcast_bpm(id, newBPM);

                if (!node->is_root) {                              // Se não for raiz, continuo subindo
                    pthread_barrier_wait(&node->parent->barrier);  // Avanço na árvore tentando liberar a barreira no pai
                }

                break;
            }
        }
    }

    return NULL;
}

// Ineficiente?
void* audio(void* arg) {
    int id = *((int*)arg);
    AudioTrack* track = track_list[id];

    play_track(track);
    while (1) {
        // Lock usado só para reler após atualização
        pthread_mutex_lock(&track->lock);
        pthread_cond_wait(&track->cond, &track->lock);  // Variável de condição acordou a thread e retomou o lock automaticamente.
        play_track(track);
        pthread_mutex_unlock(&track->lock);
    }

    return NULL;
}