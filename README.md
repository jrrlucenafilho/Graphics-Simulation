# Graphics-Simulation

Projeto final da disciplina **Introdução à Computação Gráfica** (2026.1) — UFPB  
Professor: Davi Santos
Grupo: José Ricardo Rodrigues, Ana Vitória, Leomax

---

## O que o código faz

O programa é um **visualizador 3D de modelos STL/OBJ** escrito em C++ com **OpenGL 2.1 + FreeGLUT**. Ele permite:

- **Importar modelos STL** (ASCII ou binário) ou **OBJ** (com textura referenciada pelo arquivo `.mtl`). Modelos STL são automaticamente centralizados na origem e recebem coordenadas UV geradas por projeção planar da caixa envolvente. Enquanto nenhum modelo é carregado, um bule (teapot) em wireframe é exibido como placeholder.
- **Exportar o modelo** carregado: STL ASCII quando não há textura, ou OBJ com arquivo `.mtl` e a textura copiada ao lado quando há textura aplicada.
- **Aplicar texturas** a partir de qualquer imagem (JPG/PNG), carregada com a biblioteca `stb_image` e aplicada ao modelo junto com a iluminação.
- **Manipular duas fontes de luz** (`GL_LIGHT0` e `GL_LIGHT1`), desenhadas na cena como esferas em wireframe. Clicar em uma lâmpada a seleciona (exibindo eixos XYZ como gizmo) e arrastá-la move a luz em 3D — a iluminação do modelo é atualizada em tempo real.
- **Transformar o modelo (translação, rotação e escala não uniforme)**: as teclas `1` (mover), `2` (girar) e `3` (esticar) selecionam o modo de transformação; as teclas `X/x`, `Y/y` e `Z/z` aplicam a ação do modo ativo no eixo correspondente — maiúscula no sentido positivo e minúscula no negativo; `R` restaura o modelo ao estado original. A transformação é feita com **matrizes de transformação** (`apply_model_transform()` entre `glPushMatrix`/`glPopMatrix`), sem alterar a geometria em memória; `GL_NORMALIZE` mantém a iluminação correta sob escala não uniforme, e a exportação aplica a escala atual aos vértices salvos.
- **Modo Showcase**: a câmera percorre automaticamente, durante 8 segundos, uma trajetória fechada ao redor do modelo definida por segmentos de **curvas de Bézier cúbicas** (8 pontos de controle formando um anel, com continuidade C1 obtida pela conversão de Hermite para Bézier). A curva também é desenhada na cena.
- **Cena auxiliar**: grid no chão, indicador de eixos em miniatura no canto da tela e barra de botões clicáveis com efeito de hover (Importar, Exportar, Textura, Showcase).

---

## Imagens do programa

**Visão geral:**

![Visão geral](/docs/visao_geral.gif)

**Modo Showcase (câmera percorrendo a trajetória de Bézier):**

![Showcase](/docs/showcase.gif)

**Movimentação de Luzes:**

![Luzes](/docs/lights.gif)

**Rotação em Eixo:**

![Rotação](/docs/rotation.gif)

**Mudança de Escala em Eixo:**

![Mudança de Escala](/docs/scale.gif)

**Translação em Eixo:**

![Translação](/docs/translate.gif)

**Aplicação de Textura:**

![Textura](/docs/texture.gif)

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
| Importar modelo (STL/OBJ) | botão **Importar** ou tecla `I` |
| Exportar modelo (STL ASCII ou OBJ) | botão **Exportar** ou tecla `E` |
| Carregar textura (JPG/PNG) | botão **Textura** ou tecla `T` |
| Ativar/desativar modo Showcase | botão **Showcase** ou tecla `S` |
| Selecionar modo de transformação | `1` (mover), `2` (girar), `3` (esticar) |
| Aplicar ação do modo ativo no eixo X | `X` (positivo) / `x` (negativo) |
| Aplicar ação do modo ativo no eixo Y | `Y` (positivo) / `y` (negativo) |
| Aplicar ação do modo ativo no eixo Z | `Z` (positivo) / `z` (negativo) |
| Restaurar transformações do modelo | tecla `R` |
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
│   │   ├── common.hpp               # Vec3, Vec2, Triangle + globals
│   │   └── transform.hpp/cpp        # Translação, rotação e escala não uniforme
│   ├── io/                          # Operações de entrada/saída
│   │   ├── file_dialog.hpp/cpp      # Janelas de diálogo (zenity)
│   │   ├── stl_io.hpp/cpp           # Load/save STL/OBJ, centralizar, UV
│   │   ├── texture.hpp/cpp          # Carregamento de texturas
│   │   └── stb_image.h              # Biblioteca de imagens
│   ├── render/                      # Renderização
│   │   ├── scene.hpp/cpp            # Cena 3D (grid, luzes, eixos, trajetória de Bézier)
│   │   └── ui.hpp/cpp               # Interface 2D (botões)
│   └── interaction/                 # Interação do usuário
│       └── interaction.hpp/cpp      # Seleção/arraste de lâmpadas (picking, raio-plano)
├── build/                           # Objetos compilados (gerado)
├── models/                          # Modelos STL de exemplo
├── textures/                        # Imagens de textura de exemplo
├── Makefile
└── README.md
```

---

## Principais problemas encontrados

- **Ordem de renderização e depth mask da interface 2D**: a interface (botões, textos) era encoberta (ou encobria incorretamente) elementos 3D da cena; foi necessário desativar o depth test/depth mask ao desenhar a UI e ajustar a ordem de desenho.
- **Suporte aos dois formatos de STL**: arquivos STL podem ser ASCII ou binários, com variações de formatação (espaços, tabs, `\r`); o parser precisou tratar os dois casos e normalizar as linhas.
- **Picking em 3D**: converter a posição do mouse (2D) em um raio no espaço da cena com `gluUnProject`, usado para arrastar as lâmpadas em um plano paralelo à câmera.
- **Manuseio da luzes em 3D**: Maneira para manusear as luzes de maneira mais dinâmica, sem depender tanto da perspectiva da câmera.

---

## O que pode ser melhorado (e como melhorar)

- Melhorar a interface do usuário (por exemplo, feedback visual do modo ativo e organização das opções).
- Permitir configurar pela interface a duração e o número de voltas da trajetória de Bézier do Showcase.
- Otimizar a renderização de modelos grandes usando *vertex arrays*/VBOs em vez de `glBegin`/`glEnd` (modo imediato).
- Introduzir a pintura de faces, com desfazer e salvamento das cores no arquivo exportado.

---

## Elementos de cada atividade prática

- **Atividade Prática 2 — Movimentação de objetos e uso da pilha**: transformação de objetos, matrizes e pilha de transformação.
- **Atividade Prática 3 — Uso correto de Oclusão**: ocultação de superfícies (depth test) e interface 2D.
- **Atividade Prática 4 — Fontes de luz com comportamento correto**: luzes, materiais e reposicionamento interativo.
- **Atividade Prática 5 — Utilização de imagens como texturas**: texturização de modelos.
- **Atividade Prática 6 — Utilização de Curvas**: curvas de Bézier no modo Showcase.

---

## Contribuições por integrante


| Integrante | Contribuições |
|---|---|
| José Ricardo Rodrigues | Iluminação, Importar, Exportar, Eixos, Splines  |
| Ana Vitória |Translação, Rotação, Escala, Eixos |
| Leomax | Exportação, Escala |
