#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
typedef struct AudioTrack {
    int id;                          // Identificador da thread
    int bpm;                         // Velocidade de reprodução
    int updated;                     // Flag para sincronização (espera continuada)
    double accumulated_time;         // Duração que a faixa tocou
    struct timespec last_play_time;  // Tempo em que a faixa começou a última reprodução
    pthread_mutex_t lock;            // Lock para atualização dos dados
} AudioTrack;

typedef struct TreeNode {
    int id;                     // Identificador da thread
    int is_root;                // 1 é raiz, 0 não é raiz
    int is_leaf;                // 1 é folha, 0 não é folha
    int level;                  // Qual o nível/altura do nó
    struct TreeNode* parent;    // Ponteiro para o pai (travessia bottom-up)
    struct TreeNode* left;      // Ponteiro para o filho à esquerda
    struct TreeNode* right;     // Ponteiro para o filho à direita
    pthread_barrier_t barrier;  // Barreira para sincronização
    AudioTrack track;           // Informações para thread de áudio
} TreeNode;

int N;
int LEAFS;
int NODES;

pthread_t* audio_threads;
pthread_t* sync_threads;
int* thread_ids;
TreeNode** tree;

// AudioTrack** track_list;
pthread_barrier_t global_start_barrier;
pthread_cond_t atualizar_tracks_cond = PTHREAD_COND_INITIALIZER;

int run = 1;

void* sync_thread(void* arg);
void* audio_thread(void* arg);

void build_tree() {
    for (int i = 0; i < NODES; i++) {
        TreeNode* node_ptr = malloc(sizeof(TreeNode));
        node_ptr->id = i;
        node_ptr->is_root = 0;
        node_ptr->is_leaf = 0;
        node_ptr->level = 0;
        node_ptr->parent = NULL;
        node_ptr->left = NULL;
        node_ptr->right = NULL;

        // Dados da track
        node_ptr->track.id = i;
        node_ptr->track.bpm = (80 + rand() % 41);  // Varia de 80 até 120
        node_ptr->track.updated = 0;
        node_ptr->track.accumulated_time = 0.0;
        clock_gettime(CLOCK_MONOTONIC, &(node_ptr->track.last_play_time));
        pthread_mutex_init(&(node_ptr->track.lock), NULL);

        tree[i] = node_ptr;
    }

    for (int i = 0; i < NODES; i++) {
        TreeNode* node = tree[i];

        int parent_idx = (i - 1) / 2;
        int left_idx = 2 * i + 1;
        int right_idx = 2 * i + 2;

        if (i > 0) {
            node->parent = tree[parent_idx];
            node->level = node->parent->level + 1;
        } else {
            node->level = 0;
        }

        if (left_idx < NODES) {
            node->left = tree[left_idx];
        }

        if (right_idx < NODES) {
            node->right = tree[right_idx];
        }

        node->is_root = i == 0;
        node->is_leaf = (left_idx >= NODES) || (right_idx >= NODES);

        if (!node->is_leaf) {
            pthread_barrier_init(&(node->barrier), NULL, 3);
        }
    }
    printf("[Main] Árvore construída. N=%d, NODES=%d, LEAFS=%d\n", N, NODES, LEAFS);
    printf("[Main] Dados das faixas de áudio inicializados.\n");
}

void atualizar_bpm_recursivo(TreeNode* node, int new_bpm, double new_position) {
    AudioTrack* track = &(node->track);

    pthread_mutex_lock(&track->lock);
    {
        track->bpm = new_bpm;
        track->accumulated_time = new_position;
        track->updated = 1;
        printf("[Recursao] Atualizando Track %d para %d BPM a partir de %.2fs.\n", track->id, new_bpm, new_position);
    }
    pthread_mutex_unlock(&track->lock);

    int left_idx = 2 * track->id + 1;
    int right_idx = 2 * track->id + 2;

    if (left_idx < NODES) {
        atualizar_bpm_recursivo(node->left, new_bpm, new_position);
    }

    if (right_idx < NODES) {
        atualizar_bpm_recursivo(node->right, new_bpm, new_position);
    }
}

void play_track(AudioTrack* track) {
    int target_audio = track->id;
    float speed = (float)track->bpm / 100.0f;
    speed = (speed < 0.5f) ? 0.5f : speed;

    char command[512];

    snprintf(command, sizeof(command), "kill $(cat /tmp/audio_thread_%d.pid 2>/dev/null) 2>/dev/null", track->id);
    system(command);

    // Registra o momento exato do play
    clock_gettime(CLOCK_MONOTONIC, &track->last_play_time);

    // Usa -ss para iniciar da posição acumulada
    snprintf(command, sizeof(command),
             "ffplay -nodisp -autoexit -ss %.2f -af atempo=%.2f tracks/audio%d.mp3 > /dev/null 2>&1 & echo $! > /tmp/audio_thread_%d.pid",
             track->accumulated_time, speed, target_audio, track->id);

    system(command);
}

int calculate_new_bpm(TreeNode* node) {
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

    LEAFS = 1 << N;
    NODES = (1 << (N + 1)) - 1;
    audio_threads = malloc(NODES * sizeof(pthread_t));
    sync_threads = malloc(NODES * sizeof(pthread_t));
    thread_ids = malloc(NODES * sizeof(int));
    tree = malloc(NODES * sizeof(TreeNode*));

    pthread_barrier_init(&global_start_barrier, NULL, NODES * 2);

    build_tree();

    printf("[Main] Iniciando criação das threads...\n");
    for (int i = 0; i < NODES; i++) {
        thread_ids[i] = i;
        pthread_create(&audio_threads[i], NULL, audio_thread, &thread_ids[i]);
        pthread_create(&sync_threads[i], NULL, sync_thread, &thread_ids[i]);
    }

    printf("[Main] Aguardando finalização das threads\n");
    for (int i = 0; i < NODES; i++) {
        pthread_join(sync_threads[i], NULL);
    }

    getchar();
    run = 0;
    pthread_cond_broadcast(&atualizar_tracks_cond);

    for (int i = 0; i < NODES; i++) {
        pthread_join(audio_threads[i], NULL);
    }

    for (int i = 0; i < NODES; i++) {
        if (!tree[i]->is_leaf) {
            pthread_barrier_destroy(&(tree[i]->barrier));
        }
        pthread_mutex_destroy(&(tree[i]->track.lock));
        free(tree[i]);
    }

    pthread_cond_destroy(&atualizar_tracks_cond);

    printf("[Main] Finalizando processos de áudio...\n");
    for (int i = 0; i < NODES; i++) {
        char command[128];
        snprintf(command, sizeof(command), "kill $(cat /tmp/audio_thread_%d.pid 2>/dev/null) 2>/dev/null", tree[i]->track.id);
        system(command);
    }

    free(audio_threads);
    free(sync_threads);
    free(thread_ids);
    free(tree);

    printf("[Main] Programa finalizado com sucesso.\n");
    return 0;
}

double salvar_tempo_reproducao(TreeNode* node) {
    pthread_mutex_lock(&node->track.lock);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed_real = (now.tv_sec - node->track.last_play_time.tv_sec) +
                          (now.tv_nsec - node->track.last_play_time.tv_nsec) / 1e9;
    double speed = (float)node->track.bpm / 100.0f;
    speed = (speed < 0.5f) ? 0.5f : speed;

    double parent_position = node->track.accumulated_time + (elapsed_real * speed);

    pthread_mutex_unlock(&node->track.lock);

    return parent_position;
}

void sincronizar_subarvore(TreeNode* node) {
    sleep(((N - node->level) * 2) + (rand() % 5));  // Delay para conseguir ouvir sincronizando (inversamente proporcional à altura)

    AudioTrack* left_track = &(node->left->track);
    AudioTrack* right_track = &(node->right->track);
    int newBPM = (left_track->bpm + right_track->bpm) / 2;

    printf("[Sync %d] INICIANDO SINCRONIZAÇÃO DO NOVO BPM: %d (Esq: %d, Dir: %d)\n", node->id, newBPM, left_track->bpm, right_track->bpm);
    double parent_position = salvar_tempo_reproducao(node);  // Salva o tempo de reprodução pro audio continuar de onde parou
    atualizar_bpm_recursivo(node, newBPM, parent_position);  // Primeiro propaga o BPM para toda subárvore
}

void* sync_thread(void* arg) {
    int id = *((int*)arg);
    TreeNode* node = tree[id];
    AudioTrack* track = &(node->track);

    pthread_barrier_wait(&global_start_barrier);

    printf("[Sync %d] Iniciado. %s\n", id, node->is_leaf ? "É folha." : (node->is_root ? "É raiz." : "Nó intermediário."));

    // 1. CASO BASE: um único nó que é folha e raiz ao mesmo tempo
    if (node->is_leaf && node->is_root) {
        printf("[Sync %d] É folha e raiz ao mesmo tempo (N=0). Não há com quem sincronizar. Encerrando.\n", id);
        return NULL;
    }

    // 2. SE FOR APENAS FOLHA
    if (node->is_leaf) {
        // Começa "sincronizado", acesso a barreira do pai imediamente e que começa a propagação bottom-up
        printf("[Sync %d] Esperando na barreira do pai (nó %d)...\n", id, node->parent->id);
        pthread_barrier_wait(&node->parent->barrier);
        return NULL;
    }

    // 3. SE FOR INTERMEDIÁRIO OU RAIZ

    // Primeira fase (espera para sincronização)
    printf("[Sync %d] Esperando na PRÓPRIA barreira os filhos chegarem...\n", id);
    pthread_barrier_wait(&node->barrier);  // Espero na própria barreira
    printf("[Sync %d] Barreira liberada! Vou calcular a média da minha subárvore.\n", id);

    // Segunda fase (cálculo do BPM e atualização da subárvore)
    sincronizar_subarvore(node);

    // Terceira fase (broadcast)
    printf("[Sync %d] ENVIANDO SINAL DE UPDATE\n", id);
    pthread_cond_broadcast(&atualizar_tracks_cond);  // Faz proadcast para as thread de audio reiniciarem a reprodução

    // Última fase (verificação)
    if (!node->is_root) {  // Se for nó intermediário, acesso a próxima barreira
        printf("[Sync %d] Intermediário terminou. Esperando na barreira do pai (nó %d)...\n", id, node->parent->id);
        pthread_barrier_wait(&node->parent->barrier);
    } else {  // Se for a raiz, acabou a sincronização
        printf("[Sync %d] É RAIZ. Sincronização completa alcançada.\n", id);
    }

    return NULL;
}

void* audio_thread(void* arg) {
    int id = *((int*)arg);
    TreeNode* node = tree[id];
    AudioTrack* track = &(node->track);

    printf("[Audio %d] Iniciado. BPM Inicial: %d. Indo dormir...\n", id, track->bpm);

    play_track(track);                            // Play
    pthread_barrier_wait(&global_start_barrier);  // Barreira inicial para garantir que a reprodução começa antes da sincronização

    // A unica coisa que essa thread faz é reproduzir o áudio e reiniciar a reprodução caso o BPM mude
    while (run) {
        pthread_mutex_lock(&track->lock);  // Lock para espera cotinuada
        {
            printf("[Audio %d] Aguardando broadcast...\n", id);
            while (!track->updated && run) {                              // Loop necessário para simplificar broadcast
                pthread_cond_wait(&atualizar_tracks_cond, &track->lock);  // Fica em espera até que ocorra uma nova atualização do BPM
            }
            printf("[Audio %d] Novo BPM detectado: %d\n", id, track->bpm);
            track->updated = 0;  // Flag para espera
        }
        pthread_mutex_unlock(&track->lock);

        play_track(track);  // Replay
    }

    return NULL;
}