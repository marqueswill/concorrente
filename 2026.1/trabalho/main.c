#define _XOPEN_SOURCE 600 /* Or higher */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    int id;
    int bpm;
    int updated;
    pthread_mutex_t lock;  // Lock para atualizar o bpm
    pthread_cond_t cond;   // Evita busy waiting
} AudioTrack;

// #define N 2;  // Altura da árvore
// #define LEAFS (1 << N)
// #define NODES ((1 << (N + 1)) - 1)

// pthread_t audio_threads[NODES];  // Threads que reproduzem o aúdio
// pthread_t sync_threads[NODES];   // Threads que interagem com a árvore

// int thread_ids[NODES];

// AudioTrack* track_list[NODES];
// TreeNode* tree[NODES];

int N;
int LEAFS;
int NODES;

pthread_t* audio_threads;
pthread_t* sync_threads;
int* thread_ids;
AudioTrack** track_list;
TreeNode** tree;

int run = 1;

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
        track_ptr->id = i;
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
    {
        track->bpm = new_bpm;
        track->updated = 1;
        pthread_cond_signal(&track->cond);  // aviso pra thread de audio atualizar
    }
    pthread_mutex_unlock(&track->lock);

    broadcast_bpm(left_idx, new_bpm);
    broadcast_bpm(right_idx, new_bpm);
}

void play_track(AudioTrack* track) {
    printf("THREAD %d tocando audio.", track->id);
    
    char command[512];

    // 1. Interrompe a execução anterior desta faixa específica usando pkill
    snprintf(command, sizeof(command), "pkill -f 'ffplay.*tracks/audio%d.mp3'", track->id);
    system(command);

    // 2. Calcula a taxa de velocidade.
    // Assumimos 100 BPM como velocidade normal (1.0x).
    // O filtro atempo do FFmpeg aceita valores entre 0.5 e 100.0.
    float speed = (float)track->bpm / 100.0f;
    if (speed < 0.5f) {
        speed = 0.5f;
    }

    // 3. Constrói o comando de reprodução em segundo plano (&)
    // -nodisp: desativa a janela de vídeo/espectro
    // -autoexit: fecha o processo ao terminar o áudio
    // > /dev/null 2>&1: suprime os logs do ffplay no terminal
    snprintf(command, sizeof(command),
             "ffplay -nodisp -autoexit -af atempo=%.2f tracks/audio%d.mp3 > /dev/null 2>&1 &",
             speed, track->id);

    // 4. Executa a nova faixa
    system(command);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uso: %s <altura_arvore>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);

    if (N < 0) {
        printf("Erro: N deve ser maior ou igual a zero.\n");
        return 1;
    }

    // Inicializa variáveis globais e structs
    LEAFS = 1 << N;
    NODES = (1 << (N + 1)) - 1;
    audio_threads = malloc(NODES * sizeof(pthread_t));
    sync_threads = malloc(NODES * sizeof(pthread_t));
    thread_ids = malloc(NODES * sizeof(int));
    track_list = malloc(NODES * sizeof(AudioTrack*));
    tree = malloc(NODES * sizeof(TreeNode*));

    build_tree();
    init_track_data();

    // Criar threads
    for (int i = 0; i < NODES; i++) {
        thread_ids[i] = i;
        // pthread_create(&sync_threads[i], NULL, cantor, &thread_ids[i]);
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

    while (run) {
        if (node->is_leaf) {                               // Se a thread é folha, tento sincronizar com a irmã acessando o nó do pai
            pthread_barrier_wait(&node->parent->barrier);  // Avanço na árvore tentando liberar a barreira no pai
            break;                                         // Sincronizei, em teoria essa thread não faz mais nada
        } else {
            // Se a thread não é folha:
            // Está esperando os filhos, faço nada até que os filhos chegem: acesso a barreira do nó atual
            int res = pthread_barrier_wait(&node->barrier);
            // Quando o filhos chegarem (eles acessaram a barreira pela esquerda e pela direita)
            if (res == PTHREAD_BARRIER_SERIAL_THREAD) {  // Apenas UMA das 3 threads entra neste bloco.
                //  Eu calculo a média das velocidades
                AudioTrack* left_track = track_list[node->left->id];
                AudioTrack* right_track = track_list[node->right->id];
                int newBPM = (left_track->bpm + right_track->bpm) / 2;

                //  Eu propago essa nova média para a subárvore esquerda e direita
                broadcast_bpm(id, newBPM);

                if (!node->is_root) {                              // Se não for raiz, eu tenho pa entãoi:
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
    sleep((rand() % 3) * id);
    play_track(track);
    while (run) {
        // Lock usado só para reler após atualização
        pthread_mutex_lock(&track->lock);
        {
            while (!track->updated) {                           // Espera continuada evita acordar sem sinal
                pthread_cond_wait(&track->cond, &track->lock);  // Variável de condição acordou a thread e retomou o lock automaticamente.
            }
            track->updated = 0;
        }
        pthread_mutex_unlock(&track->lock);

        play_track(track);  // Replay da música
    }

    return NULL;
}