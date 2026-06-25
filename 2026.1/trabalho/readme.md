# Trabalho Programação Concorrente 2026.1

## Identificação

Nome: Willyan Marques de Melo
Matrícula: 221020940

## Links

Link para o vídeo: https://youtu.be/hJf17JEyqvY

Link para o repositório: https://github.com/marqueswill/concorrente

## Dependências

- Sistema Operacional Ubuntu
- Compilador gcc  
- Biblioteca ffplay 

```bash
sudo apt install ffmpeg
```
## Como executar

No terminal, navegue até a pasta "trabalho" e compile o programa:

```bash
gcc ./main -o main
```

Para executá-lo, existem duas opções. A primeira é apenas passa a altura da árvore, ele irá usar os arquivos presente em trabalho/tracks

```bash
./main 0
```
Caso você queira executar com um arquivo especifico, você pode passar o caminho dele como argumento:
```bash
./main 0 caminho_arquivo.mp3
``` 

O 0 pode ser qualquer número inteiro menor que 5. Recomendo fortemente não usar nenhum valor acima disso...

