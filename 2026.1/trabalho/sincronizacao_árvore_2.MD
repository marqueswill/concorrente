# Trabalho Programação Concorrente

## Introdução

A inspiração para esse problema surgiu no algoritmo de merge sort que tem uma eficiência muito melhor se você utiliza processos de paralelismo para subdividir o problema em partes menores e processá-los simultâneamente. A ideia era fazer um problema que seguisse uma estrutura em árvore e que a sincronização ocorresse de baixo para cima (bottom-up), isso é, das folhas até as raízes.

Outra inpiração foi o fato de que muitos problemas de concorrência são difícies de visualizar e abstrair mentalmente, então seria muito interessante se fosse possível ver essa sincronização ocorrendo ou, no caso desse exercícios, ouvir as sincronização. Na teoria musical, sincronia é um conceito muito associado à harmonia, como notas de um piano ou violão sendo tocadas juntas para produzir um acorde, ou instrumentos sendo tocados dentro de um ritmo e também vozes de um coro que precisam harmonizar entre si.

Dito isso, decidi fazer um problema que envolvesse barreiras e a sincronização da velocidade de reprodução entre vozes de um coral com a restrição de que não houvesse barreira global e que a sincronização fosse sempre entre sub-árvores.

## Formalização do Problema Proposto

### Descrição do Problema

Uma música é composta por um número de faixas de áudio correspondente a uma potência de 2 ($N = 2 ^{k+1} -1 $).

Inicialmente, cada faixa começa a ser reproduzida em uma velocidade (BPM) aleatória.

A sincronização não deve interromper a reprodução do áudio. Interrupções só podem ser feitas pela própria thread de audio para tocar a faixa com o novo BPM.

O algoritmo de sincronização deve se basear em uma estrutura de Árvore Binária em Heap e cada nó da árvore representa uma thread. As sincronizações devem ocorrer das folhas até a raiz, dois nós por vez e de forma concorrente.

### Requisitos

1. **Desacoplamento Completo:** A thread de áudio deve apenas ler a velocidade de reprodução
2. **Escopo de Sincronização Estrito:** É proibido o uso de um único mutex global ou de uma única barreira global para a sincronização de todas as faixas simultaneamente. As disputas e os pontos de sincronização devem ser restritos aos pares de cada nó específico.
3. **Prevenção de Impasses:** O algoritmo deve garantir a ausência de _deadlocks_ em ambas as fases da árvore.

## Descrição do Algortimo Desenvolvido para a Solução do Problema Proposto

### Sincronização

Cada nó da árvore posssui uma barreira. As folhas possuem uma barreira de 1, pois estão localmente sincronizadas, e os demais nós possuem uma barreira de tamanho 3, uma para a thread de sincronização que ele representa e os demais para cada um de seus filhos.

#### 1. Fase de Subida (Redução Paralela Bottom-Up)

CASO BASE: um único nó que é folha e raiz ao mesmo tempo
No caso em que a árvore tem altura 0, ela só tem uma raiz, que também é uma folha. Logo não tem o que sincronizar.

Agora se a árvore tem ao menos altura 1, isto é, 3 ou mais nós, existem três possibilidades
O nó é apenas uma folha: Como é um heap, então não existem sub-arvores, portanto ele está localmente sincronizado. Basta eu acessar a barreira do próximo nó da árvore.

Se o nó é intermediário ou raiz, ele acessa a própria barreira local e precisa esperar até que os seus filhos liberem essa barreira

#### 2. Fase de Descida (Broadcast Top-Down)

Como há um desaclopamento, basicamente eu acesso os nós da sub-árvore recursivamente atualizando o valor de cada BPM para o novo valor calculado. Como para calcular a média ele precisa receber a média da própria sub-árvore, e esse valor é sempre propagado, é garantido que o valor que chegar no nó da sub-arvore já é a média calculada.

Depois de atualizar o BPM da sub-árvore toda, eu dou brodcast pras threads de audio. TODAS threads são liberadas ao mesmo tempo, as threads que não tiveram seu BPM alterado (!track->updated) continuam no loop, e as demais reiniciam sua reprodução com a nova velocidade. Agora essas threads que deram replay vão estar com o audio sincronizado, diminuindo a cacofonia.

### Comportamento Esperado

Audio começa de forma caótica e vai ficando harmonioso de forma exponencial

## Conclusão

## Referências
