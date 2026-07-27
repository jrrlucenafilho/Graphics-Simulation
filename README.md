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
- **Zenity** (`zenity`) — usado para as janelas de diálogo (Abrir/Salvar arquivos)

### Compilar

```bash
make
```

### Limpar

```bash
make clean
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

## Estrutura do projeto

```
Projeto Final/
├── src/
│   ├── main.cpp                     # Entry point + GLUT callbacks
│   ├── core/
│   │   └── common.h                 # Vec3, Vec2, Triangle + globals
│   ├── io/                          # Operações de entrada/saída
│   │   ├── file_dialog.h/cpp        # Janelas de diálogo (zenity)
│   │   ├── stl_io.h/cpp             # Load/save STL, centralizar, UV
│   │   ├── texture.h/cpp            # Carregamento de texturas
│   │   └── stb_image.h              # Biblioteca de imagens
│   ├── render/                      # Renderização
│   │   ├── scene.h/cpp              # Cena 3D (grid, luzes, eixos, spline)
│   │   └── ui.h/cpp                 # Interface 2D (botões, paleta)
│   └── interaction/                 # Interação do usuário
│       └── interaction.h/cpp        # Ray picking, arrastar lâmpadas
├── build/                           # Objetos compilados (gerado)
├── models/                          # Modelos STL de exemplo
├── Makefile
└── README.md
```

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
