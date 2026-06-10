## Exercício: Sincronização Hierárquica de uma Orquestra Dissonante

### Contexto

Uma orquestra digital é composta por 4 naipes de instrumentos: Cordas, Madeiras, Metais e Percussão. Cada naipe possui um número finito $N$ de instrumentos. Cada instrumento é representado por um par de threads executando em paralelo: uma thread responsável pelo processamento contínuo de áudio (DSP) e uma thread responsável pelo algoritmo de harmonização (sincronização). Inicialmente, cada instrumento começa a tocar em uma frequência (pitch) totalmente aleatória, gerando uma sonoridade caótica e dissonante.

---

### O Problema Concorrente e Condições de Corrida

As frequências dos instrumentos são armazenadas em um vetor global em memória compartilhada. Sem mecanismos de sincronização, o sistema apresenta os seguintes problemas:

* **Condição de Corrida:** Múltiplas threads tentam ler e escrever no mesmo índice do vetor de frequências simultaneamente, gerando inconsistência de dados.
* **Corrupção de Dados (Data Tearing):** A thread de áudio pode ler os bytes de uma frequência (tipo `double`) no exato momento em que a thread de sincronização a está atualizando, resultando em valores espúrios e estalos no som.
* **Interrupção do Fluxo Sonoro (Underrun):** Se a thread de áudio for bloqueada por travas de sincronização longas ou variáveis de condição, o buffer de som esvazia, interrompendo a execução contínua da música.

---

### Especificação do Algoritmo de Harmonização

Você deve projetar um algoritmo concorrente hierárquico que force a orquestra a convergir de forma gradual e unificada para uma frequência alvo específica ($f_{alvo} = 440.0 \text{ Hz}$). A sincronização deve ocorrer em ciclos, seguindo os requisitos abaixo:

#### 1. Fase Local (Barreira do Naipe)

Os instrumentos de um mesmo naipe devem cooperar para encontrar a harmonia interna do grupo.

* Cada thread de sincronização adiciona sua frequência atual a um acumulador local do naipe e incrementa o contador de membros prontos.
* As primeiras $N-1$ threads do naipe devem ser suspensas imediatamente, liberando a CPU.
* A última thread a chegar ao ponto de controle assume o papel de **Líder do Naipe**, calcula a frequência média do grupo e avança para a fase global.

#### 2. Fase Global (Barreira da Orquestra)

Apenas as 4 threads líderes (uma de cada naipe) participam desta etapa.

* Cada líder adiciona a média do seu naipe a um acumulador global e incrementa o contador de naipes prontos.
* Os primeiros 3 líderes ficam retidos em suspensão.
* O último líder a chegar (o **Maestro**) calcula a média absoluta da orquestra inteira ($media\_orquestra$) e aplica a seguinte fórmula de aproximação linear para o próximo ciclo:

$$f_{nova} = media\_orquestra + \alpha \cdot (f_{alvo} - media\_orquestra)$$

*Onde $\alpha$ é a taxa de convergência (ex: $0.2$).*

#### 3. Propagação Descendente e Liberação

* O Maestro atualiza a variável de comunicação global com o valor de $f_{nova}$ e acorda os outros 3 líderes.
* Cada líder, ao acordar, replica o valor de $f_{nova}$ para a estrutura local do seu naipe e acorda os respectivos instrumentos que estavam retidos na barreira local.
* Todos os instrumentos atualizam suas frequências individuais para $f_{nova}$ de forma atômica e iniciam o próximo ciclo.

---

### Requisitos Técnicos (POSIX Pthreads)

Sua solução deve atender estritamente aos seguintes critérios:

1. **Desacoplamento Completo:** A thread de áudio **nunca** deve invocar funções de bloqueio (`pthread_cond_wait`) ou travas de longa duração. Ela deve ler a frequência atual via operações atômicas ou travas exclusivas de leitura de curtíssima duração.
2. **Primitivas Obrigatórias:** A sincronização das barreiras locais e globais deve ser estruturada utilizando exclusivamente exclusão mútua (`pthread_mutex_t`) e variáveis de condição (`pthread_cond_t`). É proibido o uso de `pthread_barrier_t` nativo ou loops de espera ocupada (spinlocks).
3. **Prevenção de Impasses:** O algoritmo deve garantir a ausência de *deadlocks* tanto na subida quanto na descida da árvore de sincronização.

---

### Comportamento Auditivo Esperado (Validação)

O sucesso do algoritmo concorrente será verificado diretamente pela percepção sonora da execução:

* **Início:** Ruído dissonante generalizado (frequências dispersas).
* **Intermediário:** Formação rápida de 4 tons puros distintos (cada naipe sincroniza internamente primeiro).
* **Final:** Os 4 tons deslizam juntos e suavemente em direção ao tom uníssono e limpo de 440 Hz, eliminando completamente qualquer batimento ou oscilação acústica.