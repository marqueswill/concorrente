## Exercício: Sincronização em Árvore Binária de um Coral Dessincronizado

### Contexto

Uma música digital é composta por um número de faixas de áudio correspondente a uma potência de 2 ($N = 2^(k+1)$ -1 ). Cada faixa é controlada por um par de threads executando em paralelo: uma thread de áudio, responsável pela reprodução contínua, e uma thread de sincronização, responsável pelo alinhamento do ritmo. Inicialmente, cada faixa começa a ser reproduzida em uma velocidade aleatória e em pontos temporais distintos, gerando uma reprodução caótica e desalinhada.

### O Problema Concorrente e Condições de Corrida

As velocidades e as posições de reprodução das faixas são armazenadas em estruturas na memória compartilhada. Sem mecanismos de sincronização locais, o sistema apresenta os seguintes problemas:

- **Condição de Corrida:** Múltiplas threads tentam ler e escrever nas velocidades e posições de reprodução simultaneamente.
- **Corrupção de Dados (Data Tearing):** A thread de áudio lê dados de posicionamento incompletos no momento em que a thread de sincronização os atualiza, resultando em saltos abruptos e estalos no som.
- **Interrupção do Fluxo Sonoro (Underrun):** Se a thread de áudio for bloqueada por travas longas, o buffer de som esvazia, interrompendo a música.

### Especificação do Algoritmo de Alinhamento

O algoritmo deve ser estruturado em uma **Árvore Binária de Sincronização**, cada nó da árvore representa uma thread. As sincronização deve ocorrer das folhas até a raiz, dois nós por vez e de forma concorrente.

#### 1. Fase de Subida (Redução Paralela Bottom-Up)

As threads de sincronização percorrem a árvore das folhas até a raiz, encontrando-se aos pares em cada nó. Em cada nó:

- A thread registra a velocidade e posição de sua subárvore e assinala sua chegada.
- O par de threads sincroniza-se no nó.
- **Definição de Papéis:** Uma thread deve ser designada como **Esperadora** (fica retida no nó aguardando a descida) e a outra como **Promovida** (calcula a média das velocidades, equaliza a posição das duas subárvores, reseta o estado de controle do nó para o próximo ciclo e avança para o nó pai na hierarquia).
- A última thread a chegar realiza o cálculo da média da velocidade das subárvore (como a média vai se propagando de baixo pra cima, basta usar os valores de um dos filhos)

#### 3. Fase de Descida (Broadcast Top-Down)

Os valores unificados de velocidade e tempo de reprodução devem ser propagados em cascata da raiz até as folhas de forma imediata, **sendo proibido qualquer delay nesta fase**.

- A thread promovida e a thread esperadora liberada descem em paralelo para os seus respectivos nós de origem no nível inferior.
- Elas copiam os dados sincronizados da raiz para os nós inferiores e liberam as threads que ficaram retidas neles.
- O processo se repete exponencialmente até atingir as folhas. Ao chegarem nas folhas, todas as threads de sincronização atualizam os parâmetros das faixas de forma atômica e liberam a execução para o próximo ciclo.

### Requisitos Técnico (POSIX Pthreads)

1. **Desacoplamento Completo:** A thread de áudio (DSP) **nunca** deve invocar funções de bloqueio (`pthread_cond_wait` / `pthread_barrier_wait`) ou travas de longa duração. Ela deve ler a velocidade e a posição atualizadas via operações atômicas ou travas exclusivas de leitura de curtíssima duração.
2. **Primitivas Permitidas:** O controle de fluxo na árvore deve utilizar `pthread_mutex_t` junto a `pthread_cond_t`, ou barreiras nativas `pthread_barrier_t` com contagem igual a 2 por nó. Loops de espera ocupada (_spinlocks_) são proibidos.
3. **Escopo de Sincronização Estrito:** É proibido o uso de um único mutex global ou de uma única barreira global para a sincronização de todas as faixas simultaneamente. As disputas e os pontos de sincronização devem ser restritos aos pares de cada nó específico.
4. **Prevenção de Impasses:** O algoritmo deve garantir a ausência de _deadlocks_ em ambas as fases da árvore.

### Comportamento Auditivo Esperado

- **Início:** Uma cacofonia com faixas tocando em ritmos completamente desalinhados e velocidades atropeladas.
- **Intermediário:** À medida que as subárvores se encontram e sincronizam (processo visível caso haja delay na subida), blocos isolados de vozes começam a marchar no mesmo tempo e velocidade.
- **Final:** Após a descida instantânea, todas as threads aplicam as variáveis na mesma janela de microssegundos. O coral passa a tocar em perfeito uníssono, com todas as faixas rigorosamente na mesma velocidade e tempo.
