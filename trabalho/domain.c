#include <pthread.h>

// --- Definições de Estado para os Pedidos ---
#define NA_FILA 0     // Pedido chegou, mas ainda não tentou pegar recursos
#define ESPERANDO 1   // Pedido tentou, mas recursos não estavam livres (o "faminto" de filosofos.c)
#define PREPARANDO 2  // Pedido pegou os recursos e está "cozinhando" (o "comendo" de filosofos.c)

// --- Quantidade de Recursos ---
#define N_BOCAS_FOGAO 2
#define N_TABUAS_CORTE 1
#define N_MAX_PEDIDOS 5

typedef struct {
    // 1. O LOCK PRINCIPAL
    // Protege *todos* os campos desta struct. Análogo ao 'pthread_mutex_t lock' em filosofos_solucao.c
    pthread_mutex_t chef;

    // 2. ESTADO DOS RECURSOS (Disponibilidade)
    int bocas_fogao_disponiveis;
    int tabuas_corte_disponiveis;
    int geladeira_disponivel;  // 1 para livre, 0 para em uso
    int pia_disponivel;        // 1 para livre, 0 para em uso

    // 3. ESTADO DOS PEDIDOS
    // Array que rastreia o estado de cada thread de pedido
    // Análogo ao 'int estados[N]' em filosofos_solucao.c
    int estado_pedidos[N_MAX_PEDIDOS];  //

    // 4. "SALA DE ESPERA" DOS PEDIDOS (Variáveis de Condição)
    // Cada pedido que não consegue os recursos precisa dormir
    // Análogo ao array 'sem_t s[N]' em filosofos_solucao.c
    pthread_cond_t cond_pedido_pode_preparar[N_MAX_PEDIDOS];

} Cozinha;

// Define o que um pedido precisa (a "receita")
typedef struct {
    int id_prato;
    int precisa_fogao;      // 0 ou 1 (ou >1 se precisar de várias bocas)
    int precisa_tabua;      // 0 ou 1
    int precisa_geladeira;  // 0 ou 1
    int precisa_pia;        // 0 ou 1
} Receita;

// Esta é a struct passada para o thread do pedido
typedef struct {
    int id;                     // O ID deste pedido (de 0 a N_MAX_PEDIDOS-1)
    int paciencia_maxima;       // O tempo até o cliente ir embora
    Cozinha* cozinha;           // Ponteiro para a struct global do gerenciador
    Receita* receita_do_prato;  // A "receita" que este thread deve executar
} PedidoThread;
