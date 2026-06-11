Abaixo está a versão do exercício reestruturada para exigir a implementação de uma barreira em árvore binária.

---

## Exercício: Sincronização em Árvore Binária de uma Orquestra Dissonante

### Contexto

Uma orquestra digital é composta por um número de instrumentos correspondente a uma potência de 2 ($N = 2^k$). Cada instrumento é representado por um par de threads executando em paralelo: uma thread responsável pelo processamento contínuo de áudio (DSP) e uma thread responsável pelo algoritmo de harmonização (sincronização). Inicialmente, cada instrumento começa a tocar em uma frequência totalmente aleatória, gerando uma sonoridade caótica e dissonante.

### O Problema Concorrente e Condições de Corrida

As frequências dos instrumentos são armazenadas em estruturas na memória compartilhada. Sem mecanismos de sincronização, o sistema apresenta os seguintes problemas:

* **Condição de Corrida:** Múltiplas threads tentam ler e escrever nas frequências simultaneamente, gerando inconsistência de dados.
* **Corrupção de Dados (Data Tearing):** A thread de áudio pode ler os bytes de uma frequência no exato momento em que a thread de sincronização a está atualizando, resultando em valores espúrios e estalos no som.
* **Interrupção do Fluxo Sonoro (Underrun):** Se a thread de áudio for bloqueada por travas de sincronização longas ou variáveis de condição, o buffer de som esvazia, interrompendo a execução contínua da música.

### Especificação do Algoritmo de Harmonização

Você deve projetar um algoritmo concorrente estruturado em uma **Árvore Binária de Sincronização**, que force a orquestra a convergir de forma gradual e unificada para uma frequência alvo específica (440 Hz).

A árvore requer a alocação de $N-1$ nós de sincronização. A execução ocorre em ciclos, divididos nas seguintes etapas estritas:

#### 1. Fase de Subida (Redução em Pares)

As threads de sincronização percorrem a árvore das folhas até a raiz, encontrando-se aos pares em cada nó. Em cada nó da árvore:

* A thread bloqueia o nó, adiciona sua frequência à estrutura e assinala sua chegada.
* **Se for a primeira thread (Esperadora):** Entra em suspensão aguardando a parceira e libera a trava.
* **Se for a segunda thread (Promovida):** Calcula a média das duas frequências registradas no nó, reseta o estado do nó para o próximo ciclo e avança para o nó superior na hierarquia, levando a média calculada.

#### 2. O Ponto de Virada (Nó Raiz)

Apenas as duas últimas threads sobreviventes se encontram no topo da árvore.

* A última thread a chegar calcula a média global de toda a orquestra ($media\_global$).
* Aplica a seguinte fórmula de aproximação linear para o próximo ciclo:

$$f_{nova} = media\_global + \alpha \cdot (f_{alvo} - media\_global)$$

*Onde $\alpha$ é a taxa de convergência constante (ex: 0.2).*

* A thread grava $f_{nova}$ no nó raiz e acorda a thread esperadora da raiz.

#### 3. Fase de Descida (Propagação e Liberação)

O novo valor de frequência deve ser propagado em cascata da raiz até as folhas.

* A thread promovida e a thread esperadora recém-acordada descem para os seus respectivos nós de origem no nível inferior.
* Elas copiam $f_{nova}$ para o nó inferior e acordam as threads que ficaram retidas lá.
* O processo se repete até atingir as folhas. Todos os instrumentos atualizam suas frequências individuais para $f_{nova}$ de forma atômica e iniciam o próximo ciclo.

### Requisitos Técnicos (POSIX Pthreads)

Sua solução deve atender estritamente aos seguintes critérios:

1. **Desacoplamento Completo:** A thread de áudio **nunca** deve invocar funções de bloqueio (`pthread_cond_wait`) ou travas de longa duração. Ela deve ler a frequência atual via operações atômicas ou travas exclusivas de leitura de curtíssima duração.
2. **Primitivas Obrigatórias:** O controle de fluxo na árvore binária deve ser estruturado utilizando exclusivamente exclusão mútua (`pthread_mutex_t`) e variáveis de condição (`pthread_cond_t`) em cada nó. É proibido o uso de `pthread_barrier_t` nativo ou loops de espera ocupada (spinlocks).
3. **Escopo de Travas:** É proibido o uso de um único mutex global para a orquestra. As disputas de acesso devem ser restritas aos pares de cada nó específico.
4. **Prevenção de Impasses:** O algoritmo deve garantir a ausência de *deadlocks* tanto na subida quanto na descida da árvore.

### Comportamento Auditivo Esperado (Validação)

O sucesso do algoritmo concorrente será verificado diretamente pela percepção sonora da execução:

* **Início:** Ruído dissonante generalizado (frequências dispersas).
* **Intermediário:** A dissonância reduz pela metade a cada nível da árvore superado, conforme as frequências se mesclam em pares, criando texturas harmônicas cada vez mais limpas.
* **Final:** Todas as threads concluem a descida e deslizam juntas em direção ao tom uníssono de 440 Hz, eliminando completamente qualquer oscilação acústica.