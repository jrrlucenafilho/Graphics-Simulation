# Graphics-Simulation

Projeto final da disciplina **Introdução à Computação Gráfica** (2026.1) — UFPB  
Professor: Davi Santos

---

## O que o código faz

O programa é um **visualizador e editor 3D de modelos STL** escrito em C++ com **OpenGL 2.1 + FreeGLUT**. Ele permite:

- **Importar modelos STL** (formato ASCII ou binário). O modelo é automaticamente centralizado na origem e recebe coordenadas UV geradas por projeção esférica. Enquanto nenhum modelo é carregado, um bule (teapot) em wireframe é exibido como placeholder.
- **Exportar o modelo** carregado para um arquivo STL ASCII.
- **Aplicar texturas** a partir de qualquer imagem (JPG/PNG), carregada com a biblioteca `stb_image` e aplicada ao modelo junto com a iluminação.
- **Pintar faces do modelo**: no modo "Pintar", uma paleta de cores (vermelho, verde e azul) aparece na tela e cada clique — ou arrasto — sobre o modelo pinta o triângulo sob o cursor. A seleção do triângulo é feita por **ray casting** (lançamento de um raio da câmera pela posição do mouse e teste de interseção raio-triângulo com o algoritmo de Möller–Trumbore).
- **Manipular duas fontes de luz** (`GL_LIGHT0` e `GL_LIGHT1`), desenhadas na cena como esferas em wireframe. Clicar em uma lâmpada a seleciona (exibindo eixos XYZ como gizmo) e arrastá-la move a luz em 3D — a iluminação do modelo é atualizada em tempo real.
- **Esticar o objeto (escala não uniforme)**: as teclas `X/x`, `Y/y` e `Z/z` esticam ou encolhem o modelo em cada eixo independentemente. A transformação é feita com **matrizes de transformação** (`glScalef` entre `glPushMatrix`/`glPopMatrix`), sem alterar a geometria em memória; `GL_NORMALIZE` mantém a iluminação correta sob escala não uniforme, e a exportação aplica a escala atual aos vértices salvos.
- **Modo Showcase**: a câmera percorre automaticamente, durante 8 segundos, uma trajetória fechada ao redor do modelo definida por uma spline (Catmull-Rom), que também é desenhada na cena.
- **Cena auxiliar**: grid no chão, indicador de eixos em miniatura no canto da tela e barra de botões clicáveis com efeito de hover (Importar, Exportar, Textura, Pintar, Showcase).

---

## Imagens do programa

<!-- Substitua os placeholders abaixo por capturas de tela ou GIFs do programa em execução.
     Sugestão: salve as imagens em uma pasta docs/ e ajuste os caminhos. -->

**Visão geral:**

![Visão geral](/docs/visao_geral.gif)

**Pintura de faces com a paleta de cores:**

![Modo pintura](docs/pintura.png)

**Modelo com textura aplicada:**

![Textura](docs/textura.png)

**Manipulação das luzes (lâmpada selecionada com gizmo):**

![Luzes](docs/luzes.png)

**Modo Showcase (câmera percorrendo a spline):**

![Showcase](/docs/showcase.gif)

---

## Instruções de como compilar e executar

O programa roda em **Linux** (nativamente) e em **Windows** (via WSL, recomendado). As dependências são:

- Compilador **g++** e **make**
- **OpenGL** e **GLU** (Mesa)
- **FreeGLUT**
- **Zenity** — usado apenas para as janelas de diálogo de Abrir/Salvar arquivos

### Linux

Instale as dependências de acordo com a sua distribuição:

```bash
# Debian / Ubuntu / Mint / Pop!_OS
sudo apt install build-essential libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev zenity

# Fedora / RHEL / Rocky / AlmaLinux
sudo dnf install gcc-c++ make mesa-libGL-devel mesa-libGLU-devel freeglut-devel zenity

# Arch / Manjaro
sudo pacman -S base-devel mesa glu freeglut zenity
```

Depois, na raiz do repositório:

```bash
make          # compila e gera o executável ./stl_viewer
make clean    # (opcional) remove os artefatos de compilação
```

### Windows (via WSL — recomendado)

No Windows 10 (build 19044+) ou Windows 11, o WSL já inclui suporte a aplicativos gráficos (WSLg), então o programa abre em uma janela normal do Windows sem configuração extra.

1. Abra o **PowerShell como administrador** e instale o WSL com Ubuntu:

   ```powershell
   wsl --install
   ```

   Reinicie o computador se for solicitado e crie seu usuário Linux na primeira execução.

2. Abra o terminal do **Ubuntu (WSL)** e instale as dependências:

   ```bash
   sudo apt update
   sudo apt install build-essential libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev zenity
   ```

3. Clone (ou acesse) o repositório dentro do WSL e compile:

   ```bash
   git clone <url-do-repositorio>
   cd Graphics-Simulation
   make
   ```

> Em versões antigas do Windows 10 sem WSLg, é necessário um servidor X (por exemplo, VcXsrv) e exportar `DISPLAY` antes de executar. Alternativamente, o projeto pode ser compilado nativamente com MSYS2/MinGW (pacotes `mingw-w64-x86_64-gcc`, `make` e `mingw-w64-x86_64-freeglut`); nesse caso, sem o Zenity, use o carregamento de modelos pela linha de comando.

### Executar

Na raiz do repositório (Linux ou terminal do WSL):

```bash
./stl_viewer                                    # inicia sem modelo (teapot placeholder)
./stl_viewer models/Stanford_Bunny_sample.stl   # carrega um modelo direto da linha de comando
```

Há modelos STL de exemplo em `models/` e imagens de textura em `textures/`.

> **Nota:** o programa precisa de um ambiente gráfico. O Zenity é necessário apenas para os diálogos de arquivo dos botões Importar/Exportar/Textura — carregar um modelo pela linha de comando funciona sem ele.

### Controles

| Ação | Comando |
|---|---|
| Importar modelo STL | botão **Importar** ou tecla `I` |
| Exportar modelo para STL ASCII | botão **Exportar** ou tecla `E` |
| Carregar textura (JPG/PNG) | botão **Textura** ou tecla `T` |
| Ativar/desativar modo pintura | botão **Pintar** ou tecla `P` (escolha a cor na paleta e clique/arraste sobre o modelo) |
| Ativar/desativar modo Showcase | botão **Showcase** ou tecla `S` |
| Esticar/encolher o objeto no eixo X | teclas `X` (estica) / `x` (encolhe) |
| Esticar/encolher o objeto no eixo Y | teclas `Y` (estica) / `y` (encolhe) |
| Esticar/encolher o objeto no eixo Z | teclas `Z` (estica) / `z` (encolhe) |
| Rotacionar a cena | arrastar com o botão esquerdo do mouse |
| Zoom | scroll do mouse |
| Selecionar/mover uma lâmpada | clicar na esfera da lâmpada e arrastar; clicar de novo (ou em área vazia) desseleciona |
| Sair | `ESC` |

---

## Estrutura do projeto

```
Graphics-Simulation/
├── src/
│   ├── main.cpp                     # Entry point + GLUT callbacks
│   ├── core/
│   │   ├── common.hpp                 # Vec3, Vec2, Triangle + globals
│   │   └── transform.hpp/cpp          # Escala não uniforme (esticar por eixo)
│   ├── io/                          # Operações de entrada/saída
│   │   ├── file_dialog.hpp/cpp        # Janelas de diálogo (zenity)
│   │   ├── stl_io.hpp/cpp             # Load/save STL, centralizar, UV
│   │   ├── texture.hpp/cpp            # Carregamento de texturas
│   │   └── stb_image.h              # Biblioteca de imagens
│   ├── render/                      # Renderização
│   │   ├── scene.hpp/cpp              # Cena 3D (grid, luzes, eixos, spline)
│   │   └── ui.hpp/cpp                 # Interface 2D (botões, paleta)
│   └── interaction/                 # Interação do usuário
│       └── interaction.hpp/cpp        # Ray picking, arrastar lâmpadas
├── build/                           # Objetos compilados (gerado)
├── models/                          # Modelos STL de exemplo
├── textures/                        # Imagens de textura de exemplo
├── Makefile
└── README.md
```

---

## Principais problemas encontrados

- **Ordem de renderização e depth mask da interface 2D**: a paleta de cores era encoberta (ou encobria incorretamente) elementos 3D da cena; foi necessário desativar o depth test/depth mask ao desenhar a UI e ajustar a ordem de desenho.
- **Suporte aos dois formatos de STL**: arquivos STL podem ser ASCII ou binários, com variações de formatação (espaços, tabs, `\r`); o parser precisou tratar os dois casos e normalizar as linhas.
- **Picking em 3D**: converter a posição do mouse (2D) em um raio no espaço da cena com `gluUnProject`, tanto para pintar triângulos quanto para arrastar as lâmpadas em um plano paralelo à câmera.
<!-- Adicionem aqui outros problemas reais enfrentados pelo grupo (desempenho, integração, etc.) -->

---

## O que pode ser melhorado (e como melhorar)

- Melhorar a interface do usuário (por exemplo, paleta com mais cores e feedback visual do modo ativo).
- Substituir a spline Catmull-Rom do Showcase por segmentos de **curvas de Bézier** explícitos, atendendo literalmente ao requisito da Atividade Prática 6.
- Otimizar a renderização de modelos grandes usando *vertex arrays*/VBOs em vez de `glBegin`/`glEnd` (modo imediato).
- Permitir desfazer a pintura de faces e salvar as cores pintadas no arquivo exportado.

---

## Os elementos de cada atividade prática

- **Atividade Prática 2 — Movimentação de objetos e uso da pilha**: rotação e zoom da cena via mouse; arrasto das lâmpadas em 3D; escala não uniforme do modelo com matrizes de transformação (`glScalef` na pilha, teclas `X/x`, `Y/y`, `Z/z`); uso de `glPushMatrix`/`glPopMatrix` para desenhar lâmpadas, gizmos de eixos e a UI 2D sem afetar as transformações da cena.
- **Atividade Prática 3 — Uso correto de Oclusão**: depth test (`GL_DEPTH_TEST`) habilitado para a cena 3D, com controle explícito de depth mask ao desenhar elementos de interface (paleta, botões, indicador de eixos).
- **Atividade Prática 4 — Fontes de luz com comportamento correto**: duas luzes posicionais (`GL_LIGHT0`/`GL_LIGHT1`) com componentes difusa e especular, materiais com brilho (shininess), e reposicionamento interativo das luzes com atualização da iluminação em tempo real.
- **Atividade Prática 5 — Utilização de imagens como texturas**: carregamento de imagens JPG/PNG com `stb_image`, geração de coordenadas UV por projeção esférica e mapeamento da textura sobre o modelo combinado com a iluminação.
- **Atividade Prática 6 — Utilização de Curvas**: trajetória de câmera do modo Showcase definida por uma spline cúbica por partes (Catmull-Rom) com 8 pontos de controle, avaliada e desenhada na cena. <!-- TODO: converter para segmentos de Bézier para atender literalmente ao enunciado -->

---

## O que cada integrante fez

| Integrante | Contribuições |
|---|---|
| José Ricardo Rodrigues | |
| Ana Vitória | |
| Leomax | |
