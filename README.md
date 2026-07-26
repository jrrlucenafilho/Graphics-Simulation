# Graphics-Simulation

Projeto final da disciplina **Introdução à Computação Gráfica** (2026.1) — UFPB  
Professor: Davi Santos

---

## O que o código faz

Tema e funcionamento geral do programa

---

## Imagem do programa

<!-- Adicione aqui uma captura de tela ou gif do programa em execução -->
```
![Screenshot](screenshot.png)
```

---

## Instruções de como compilar e executar

### Dependências

- Compilador **g++** (ou clang++)
- **OpenGL** (`libgl1-mesa-dev`)
- **GLU** (`libglu1-mesa-dev`)
- **FreeGLUT** (`freeglut3-dev`)

### Compilar

```bash
g++ -o stl_viewer main.cpp -lGL -lGLU -lglut
```

### Executar

```bash
./stl_viewer                  # sem modelo (teapot placeholder)
./stl_viewer modelo.stl       # carregar modelo direto da linha de comando
```

Use o botão **Importar** (ou tecla `I`) para selecionar um arquivo STL.  
Use o botão **Exportar** (ou tecla `E`) para salvar o modelo carregado.  
Arraste o mouse para rotacionar, scroll para zoom, tecla `R` para resetar a visualização e `ESC` para sair.

---

## Principais problemas encontrados

Liste aqui os principais desafios enfrentados durante o desenvolvimento, como:
- Dificuldades com transformações geométricas;
- Problemas de desempenho;
- Erros de iluminação ou textura;
- Integração entre os membros do grupo;
- Outros obstáculos técnicos relevantes.

---

## O que pode ser melhorado (e como melhorar)

- Melhorar a interface do usuário.

---

## Os elementos de cada atividade prática

Descreva quais elementos de cada atividade prática da disciplina foram incorporados no projeto:

- Atividade Prática 2: Movimentação de objetos e uso da pilha
- Atividade Prática 3: Uso correto de Oclusão
- Atividade Prática 4: Fontes de luz com comportamento correto
- Atividade Prática 5: Utilização de imagens como texturas
- Atividade Prática 6: Utilização de Curvas de Bèzier

---

## O que cada integrante fez

| Integrante | Contribuições |
|---|---|
| José Ricardo Rodrigues | |
| Ana Vitória | |
| Leomax | |
