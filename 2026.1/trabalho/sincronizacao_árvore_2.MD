## Exercício: Sincronização em Árvore Binária de um Coral Dessincronizado

### Inspiração
A inspiração para esse problema surgiu no algoritmo de merge sort que tem uma eficiência muito melhor se você utiliza processos de paralelismo para subdividir o problema em partes menores e processá-los simultâneamente. A ideia era fazer um problema que seguisse uma estrutura em árvore e que a sincronização ocorresse de baixo para cima (bottom-up), isso é, das folhas até as raízes. 
Outra inpiração foi o fato de que muitos problemas de concorrência são difícies de visualizar e abstrair mentalmente, então seria muito interessante se fosse possível ver essa sincronização ocorrendo ou, no caso desse exercícios, ouvir as sincronização. Na teoria musical, sincronia é um conceito muito associado à harmonia, como notas de um piano ou violão sendo tocadas juntas para produzir um acorde, ou instrumentos sendo tocados dentro de um ritmo e também vozes de um coro que precisam harmonizar entre si.
Dito isso, decidi fazer um problema que envolvesse barreiras e a sincronização da velocidade de reprodução entre vozes de um coral com a restrição de que não houvesse barreira global e que a sincronização fosse sempre entre sub-árvores.

### Contexto

Uma música digital é composta por um número de faixas de áudio correspondente a uma potência de 2 ($N = 2^(k+1)$ -1 ). 

Cada faixa é controlada por um par de threads executando em paralelo: uma thread de áudio, responsável pela reprodução ineterrupta do arquivo de áudio e uma thread de sincronização responsável. 

Inicialmente, cada faixa começa a ser reproduzida em uma velocidade (BPM) aleatória.

O algoritmo de sincronização deve se basear em uma estrutura de Árvore Binária em Heap e cada nó da árvore representa uma thread. As sincronizações devem ocorrer das folhas até a raiz, dois nós por vez e de forma concorrente.


### Problemas de Sincronização

### Algoritmo de Sincronização

Como a thread de reprodução e a de sincronização estão separadas
Duas estruturas de árvore: uma para armazenar os BPMs e dados para reprodução e outra para armazernar os dados relacionado ao estado das threads de sincronização.
Separação não é necessária, mas ajuda no entedimento

Os dados de BPM são atualizado -> pode causar 

#### 1. Fase de Subida (Redução Paralela Bottom-Up)


#### 2. Fase de Descida (Broadcast Top-Down)


### Requisitos Técnico (POSIX Pthreads)

1. **Desacoplamento Completo:** A thread de áudio (DSP) **nunca** deve invocar funções de bloqueio (`pthread_cond_wait` / `pthread_barrier_wait`) ou travas de longa duração. Ela deve ler a velocidade atualizada utilizando travas de baixa duração.
2. **Escopo de Sincronização Estrito:** É proibido o uso de um único mutex global ou de uma única barreira global para a sincronização de todas as faixas simultaneamente. As disputas e os pontos de sincronização devem ser restritos aos pares de cada nó específico.
3. **Prevenção de Impasses:** O algoritmo deve garantir a ausência de _deadlocks_ em ambas as fases da árvore.

### Comportamento Esperado
