#define _XOPEN_SOURCE 600 /* Or higher */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct TreeNode {
    int id;
    int is_root;
    int is_leaf;
    int level;
    struct TreeNode* parent;
    struct TreeNode* left;
    struct TreeNode* right;
    pthread_barrier_t barrier;
} TreeNode;

typedef struct AudioTrack {
    int id;
    int bpm;
    int updated;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} AudioTrack;

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
        node_ptr->level = 0;
        node_ptr->parent = NULL;
        node_ptr->left = NULL;
        node_ptr->right = NULL;

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
        pthread_barrier_init(&(node->barrier), NULL, node->is_leaf ? 1 : 3);
    }
    printf("[Main] Árvore construída. N=%d, NODES=%d, LEAFS=%d\n", N, NODES, LEAFS);
}

void init_track_data() {
    for (int i = 0; i < NODES; i++) {
        AudioTrack* track_ptr = malloc(sizeof(AudioTrack));
        track_ptr->id = i;
        track_ptr->bpm = (rand() % 200) + 1;

        pthread_mutex_init(&(track_ptr->lock), NULL);
        pthread_cond_init(&(track_ptr->cond), NULL);

        track_list[i] = track_ptr;
    }
    printf("[Main] Dados das faixas de áudio inicializados.\n");
}

void broadcast_bpm(int track_id, int new_bpm) {
    AudioTrack* track = track_list[track_id];

    pthread_mutex_lock(&track->lock);
    {
        track->bpm = new_bpm;
        printf("[Broadcast] Atualizando Track %d para %d BPM e enviando sinal.\n", track_id, new_bpm);
        pthread_cond_signal(&track->cond);
    }
    pthread_mutex_unlock(&track->lock);

    int left_idx = 2 * track_id + 1;
    int right_idx = 2 * track_id + 2;

    if (left_idx < NODES) {
        broadcast_bpm(left_idx, new_bpm);
    }

    if (right_idx < NODES) {
        broadcast_bpm(right_idx, new_bpm);
    }
}

void play_track(AudioTrack* track) {
    int target_audio = 0;
    float speed = (float)track->bpm / 100.0f;
    if (speed < 0.5f) {
        speed = 0.5f;
    }

    char command[512];

    // 1. LER O PID E MATAR O PROCESSO ANTERIOR
    // Tenta ler o arquivo com o PID exclusivo desta thread e matar apenas ele.
    // Os "2>/dev/null" impedem que erros sujem o terminal na primeira vez que rodar (quando o PID ainda não existe).
    snprintf(command, sizeof(command), "kill $(cat /tmp/audio_thread_%d.pid 2>/dev/null) 2>/dev/null", track->id);
    system(command);

    // 2. TOCAR O ÁUDIO ÚNICO E SALVAR O NOVO PID
    // Substitua "tracks/audio_unico.mp3" pelo nome real do seu arquivo.
    // O comando "echo $! > /tmp/..." pega o PID do processo em background (&) e salva no txt.
    snprintf(command, sizeof(command),
             "ffplay -nodisp -autoexit -af atempo=%.2f tracks/audio%d.mp3 > /dev/null 2>&1 & echo $! > /tmp/audio_thread_%d.pid",
             speed, target_audio, track->id);

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

    LEAFS = 1 << N;
    NODES = (1 << (N + 1)) - 1;
    audio_threads = malloc(NODES * sizeof(pthread_t));
    sync_threads = malloc(NODES * sizeof(pthread_t));
    thread_ids = malloc(NODES * sizeof(int));
    track_list = malloc(NODES * sizeof(AudioTrack*));
    tree = malloc(NODES * sizeof(TreeNode*));

    build_tree();
    init_track_data();

    printf("[Main] Iniciando criação das threads...\n");
    for (int i = 0; i < NODES; i++) {
        thread_ids[i] = i;
        // ATENÇÃO: Descomentei a linha abaixo para evitar segfault no join
        pthread_create(&sync_threads[i], NULL, cantor, &thread_ids[i]);
        pthread_create(&audio_threads[i], NULL, audio, &thread_ids[i]);
    }

    printf("[Main] Aguardando finalização das threads\n");
    for (int i = 0; i < NODES; i++) {
        pthread_join(sync_threads[i], NULL);
    }

    for (int i = 0; i < NODES; i++) {
        pthread_join(audio_threads[i], NULL);
    }

    for (int i = 0; i < NODES; i++) {
        free(tree[i]);
    }

    for (int i = 0; i < NODES; i++) {
        pthread_mutex_destroy(&(track_list[i]->lock));
        pthread_cond_destroy(&(track_list[i]->cond));
        free(track_list[i]);
    }

    printf("[Main] Programa finalizado com sucesso.\n");
    return 0;
}

void* cantor(void* arg) {
    int id = *((int*)arg);
    TreeNode* node = tree[id];
    AudioTrack* track = track_list[id];

    // printf("___________[Cantor %d] is_root = %d, is_leaf = %d\n", node->id, node->is_root, node->is_leaf);
    printf("[Cantor %d] Iniciado. %s\n", id, node->is_leaf ? "É folha." : (node->is_root ? "É raiz." : "Nó intermediário."));

    while (run) {
        if (!node->is_leaf) {
            printf("[Cantor %d] Esperando na PRÓPRIA barreira os filhos chegarem...\n", id);
            int res = pthread_barrier_wait(&node->barrier);

            printf("[Cantor %d] Barreira liberada! Vou calcular a média da minha subárvore.\n", id);
            AudioTrack* left_track = track_list[node->left->id];
            AudioTrack* right_track = track_list[node->right->id];
            int newBPM = (left_track->bpm + right_track->bpm) / 2;

            printf("[Cantor %d] Calculou novo BPM: %d (Esq: %d, Dir: %d)\n", id, newBPM, left_track->bpm, right_track->bpm);

            sleep((rand() % 6) + node->id + N - node->level + 1);  // Delay para conseguir ouvir sincronizandos, quando mais proximo da raiz mais lento
            broadcast_bpm(id, newBPM);

            if (!node->is_root) {
                printf("[Cantor %d] É INTERMEDIÁRIO. Sincronização parcial alcançada. Agora esperando na barreira do pai (nó %d)...\n", id, node->parent->id);
                pthread_barrier_wait(&node->parent->barrier);
                printf("[Cantor %d] Passou da barreira do pai.\n", id);
            } else {
                printf("[Cantor %d] É RAIZ. Sincronização completa alcançada.\n", id);
            }

            break;
        }

        if (!node->is_root) {  // É apenas folha
            printf("[Cantor %d] Esperando na barreira do pai (nó %d)...\n", id, node->parent->id);
            pthread_barrier_wait(&node->parent->barrier);
            printf("[Cantor %d] Passou da barreira do pai. Encerrando ciclo.\n", id);
        } else {  // É folha e raiz
            printf("[Cantor %d] É folha e raiz ao mesmo tempo (N=0). Não há com quem sincronizar. Encerrando.\n", id);
        }

        break;
    }

    return NULL;
}

void* audio(void* arg) {
    int id = *((int*)arg);
    AudioTrack* track = track_list[id];

    printf("[Audio %d] Iniciado. BPM Inicial: %d. Indo dormir...\n", id, track->bpm);

    int bpm_anterior = track->bpm;

    play_track(track);

    while (run) {
        pthread_mutex_lock(&track->lock);
        {
            printf("[Audio %d] Aguardando broadcast...\n", id);
            while (track->bpm == bpm_anterior && run) {
                pthread_cond_wait(&track->cond, &track->lock);
            }
            bpm_anterior = track->bpm;
            printf("[Audio %d] Novo BPM detectado: %d\n", id, track->bpm);
        }
        pthread_mutex_unlock(&track->lock);

        play_track(track);
    }

    return NULL;
}