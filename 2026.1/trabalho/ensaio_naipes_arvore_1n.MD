## Exercício: Sincronização em Árvore Hierárquica (1 ← 4 ← N) de uma Orquestra Dissonante

### Contexto

Uma orquestra digital é composta por 4 naipes de instrumentos: Cordas, Madeiras, Metais e Percussão. Cada naipe possui um número finito $N$ de instrumentos. Cada instrumento é representado por um par de threads executando em paralelo: uma thread responsável pelo processamento contínuo de áudio (DSP) e uma thread responsável pelo algoritmo de harmonização (sincronização). Inicialmente, cada instrumento começa a jogar em uma frequência (pitch) totalmente aleatória, gerando uma sonoridade caótica e dissonante.

---

### O Problema Concorrente e Condições de Corrida

As frequências dos instrumentos são geradas e manipuladas em memória compartilhada. Sem mecanismos de sincronização, o sistema apresenta os seguintes problemas:

* **Condição de Corrida:** Múltiplas threads tentam ler e escrever nas frequências simultaneamente, gerando inconsistência de dados.
* **Corrupção de Dados (Data Tearing):** A thread de áudio pode ler os bytes de uma frequência no exato momento em que a thread de sincronização a está atualizando, resultando em valores espúrios e estalos no som.
* **Interrupção do Fluxo Sonoro (Underrun):** Se a thread de áudio for bloqueada por travas de sincronização de longa duração, o buffer de som esvazia, interrompendo a execução contínua da música.

---

### Especificação do Algoritmo de Harmonização

Você deve projetar um algoritmo concorrente estruturado em uma **Árvore Hierárquica de 3 Níveis (Raiz ← Naipes ← Instrumentos)** que force a orquestra a convergir de forma gradual e unificada para uma frequência alvo específica ($f_{alvo} = 440.0 \text{ Hz}$).

A árvore de sincronização é composta por exatamente 5 nós de controle na memória compartilhada: 4 nós intermediários (um para cada naipe) e 1 nó raiz (orquestra). O algoritmo opera em ciclos baseados em duas fases estritas:

```
Nível 1 (Raiz):          [ Nó Raiz (Orquestra) ]
                                 ▲
Nível 2 (Naipes):   [Cordas] [Madeiras] [Metais] [Percussão]
                                 ▲
Nível 3 (Inst.):     [N th]   [N th]     [N th]   [N th]

```

#### 1. Fase de Subida (Redução N → 4 → 1)

* **Nível 3 para Nível 2 (N → 4):** Cada thread de instrumento acessa o nó correspondente ao seu naipe. Ela adiciona sua frequência atual ao acumulador do nó e incrementa o contador de chegadas. As primeiras $N-1$ threads de cada naipe bloqueiam e liberam a CPU. A última thread a chegar assume o papel de **Líder do Naipe**, calcula a frequência média do grupo e avança para o nível superior.
* **Nível 2 para Nível 1 (4 → 1):** As 4 threads líderes acessam o nó raiz da árvore. Cada líder adiciona a média do seu naipe ao acumulador da raiz e incrementa as chegadas. Os primeiros 3 líderes bloqueiam. O último líder a chegar assume o papel de **Maestro**.

#### 2. O Ponto de Virada (Nó Raiz)

O Maestro, isolado na região crítica do nó raiz, consolida os dados de toda a árvore:

* Calcula a média global com base nas médias dos 4 naipes ($media\_global$).
* Aplica a seguinte fórmula de aproximação linear para o próximo ciclo:

$$f_{nova} = media\_global + \alpha \cdot (f_{alvo} - media\_global)$$

*Onde $\alpha$ é a taxa de convergência constante (ex: 0.2).*

* O Maestro armazena o valor de $f_{nova}$ no nó raiz e inicia a fase de descida.

#### 3. Fase de Descida (Propagação e Liberação 1 → 4 → N)

* **Nível 1 para Nível 2 (1 → 4):** O Maestro acorda os outros 3 líderes retidos no nó raiz. Todos os 4 líderes copiam o valor de $f_{nova}$ da raiz para as estruturas de seus respectivos nós de naipe.
* **Nível 2 para Nível 3 (4 → N):** Cada líder acorda todos os instrumentos do seu próprio naipe que estavam retidos no nó local.
* **Atualização:** Todos os instrumentos acordam, atualizam suas frequências individuais para $f_{nova}$ de forma atômica e iniciam o próximo ciclo de execução.

---

### Requisitos Técnicos (POSIX Pthreads)

Sua solução deve atender estritamente aos seguintes critérios:

1. **Desacoplamento Completo:** A thread de áudio **nunca** deve invocar funções de bloqueio (`pthread_cond_wait`) ou travas de longa duração. Ela deve ler a frequência atual via operações atômicas ou travas exclusivas de leitura de curtíssima duração.
2. **Primitivas Obrigatórias:** O controle de fluxo em cada um dos 5 nós da árvore deve ser estruturado utilizando exclusivamente exclusão mútua (`pthread_mutex_t`) e variáveis de condição (`pthread_cond_t`). É proibido o uso de `pthread_barrier_t` nativo ou loops de espera ocupada (spinlocks).
3. **Escopo de Travas:** É proibido o uso de um único mutex global para gerenciar a árvore inteira. As disputas de acesso devem ocorrer estritamente dentro do mutex de cada nó específico (isolando a disputa por naipe).
4. **Prevenção de Impasses:** O algoritmo deve garantir a ausência de *deadlocks* tanto na subida quanto na descida da árvore.

---

### Comportamento Auditivo Esperado (Validação)

O sucesso do algoritmo concorrente será verificado diretamente pela percepção sonora da execução:

* **Início:** Ruído dissonante generalizado (frequências dispersas).
* **Intermediário:** Formação de 4 tons puros e distintos de forma síncrona a cada ciclo, correspondendo à consolidação dos nós de nível 2.
* **Final:** Os 4 tons remanescentes deslizam juntos de forma unificada em direção ao tom de 440 Hz, estabilizando sem oscilações acústicas ou estalos.