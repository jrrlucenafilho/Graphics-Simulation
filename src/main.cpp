// main.cpp — Visualizador 3D de modelos STL.
//
// Ponto de entrada da aplicação. Configura a janela GLUT, registra os
// callbacks de desenho/interação e delega as tarefas aos módulos:
//   - io/stl_io:   carregar/exportar modelos e pós-processar a malha;
//   - io/texture:  carregar texturas;
//   - core/transform: escala não uniforme por teclado;
//   - render/scene e render/ui: desenhar a cena e a interface;
//   - interaction: converter mouse em coordenadas 3D.
// Os callbacks (mouse, motion, keyboard, idle, display) mantêm o estado global
// da aplicação, definido em core/common.h e iniciado aqui.

#include "core/common.h"
#include "core/transform.h"
#include "interaction/interaction.h"
#include "io/file_dialog.h"
#include "io/stl_io.h"
#include "io/texture.h"
#include "render/scene.h"
#include <cstdio>
#include <cstdlib>

// Tamanho inicial da janela antes de qualquer redimensionamento.
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

// ---------------------------------------------------------------------------
// Estado global da aplicação (declarado em core/common.h).
// ---------------------------------------------------------------------------

// Malha do modelo carregado e flag de disponibilidade.
std::vector<Triangle> g_triangles;
bool g_model_loaded = false;
// Rotação atual da câmera (controlada pelo arraste do mouse).
float g_rot_x = 0.0f, g_rot_y = 0.0f;

// Estado do arraste: posição anterior do mouse, se há arrasto ativo, zoom e
// tamanho atual da janela.
int g_last_mx = 0, g_last_my = 0;
bool g_dragging = false;
float g_zoom = 1.0f;
int g_win_w = WINDOW_WIDTH, g_win_h = WINDOW_HEIGHT;

// Destaque (hover) dos botões da interface.
bool g_import_hover = false;
bool g_export_hover = false;
bool g_texture_hover = false;

// Textura carregada para aplicar sobre o modelo.
GLuint g_texture_id = 0;
bool g_texture_loaded = false;

// Modo vitrine: a câmera percorre uma curva ao redor do modelo durante
// g_showcase_duration segundos, e o temporizador avança no callback idle.
bool g_showcase_hover = false;
bool g_showcase_active = false;
float g_showcase_time = 0.0f;
float g_showcase_duration = 8.0f;

// Posições das luzes da cena e índice da lâmpada selecionada (-1 = nenhuma).
std::vector<Vec3> g_lamp_positions;
int g_selected_lamp = -1;

// ---------------------------------------------------------------------------
// Callbacks do GLUT
// ---------------------------------------------------------------------------

// Chamado quando a janela precisa ser redesenhada: apenas invoca a
// renderização da cena (definida em render/scene.cpp).
static void display() { render_scene(); }

// Chamado quando a janela é redimensionada: atualiza as dimensões globais e o
// viewport do OpenGL para acompanhar o novo tamanho.
static void reshape(int w, int h) {
  g_win_w = w;
  g_win_h = h;
  glViewport(0, 0, w, h);
}

// Chamado continuamente quando não há eventos: avança o cronômetro do modo
// vitrine e encerra a animação quando o tempo acaba.
static void idle() {
  if (g_showcase_active) {
    g_showcase_time += 0.016f;
    if (g_showcase_time >= g_showcase_duration) {
      g_showcase_active = false;
      g_showcase_time = 0.0f;
    }
    glutPostRedisplay();
  }
}

// Trata clique do mouse. Regiões dos botões são definidas por coordenadas de
// tela fixas (combinadas com as posições usadas em render/scene.cpp). As ações
// de cada botão (importar, exportar, textura, vitrine) delegam aos módulos
// correspondentes. Fora dos botões, o clique pode selecionar/arrastar uma
// lâmpada ou iniciar a rotação da câmera; a roda do mouse ajusta o zoom.
static void mouse(int button, int state, int x, int y) {
  // Regiões clicáveis dos botões (mesmas coordenadas do desenho em scene.cpp).
  bool over_import = (x >= 20 && x <= 140 && y >= 50 && y <= 85);
  bool over_export = (x >= 150 && x <= 270 && y >= 50 && y <= 85);
  bool over_texture = (x >= 280 && x <= 400 && y >= 50 && y <= 85);
  bool over_showcase = (x >= 410 && x <= 530 && y >= 50 && y <= 85);

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    if (over_import) {
      // Interrompe a vitrine e abre o diálogo para escolher um STL. Após
      // carregar, orienta, centraliza e gera UVs para a textura.
      g_showcase_active = false;
      g_showcase_time = 0.0f;
      std::string path = open_file_dialog();
      if (!path.empty()) {
        if (load_stl(path)) {
          orient_model();
          center_model();
          generate_uv_coords();
          printf("Modelo carregado: %s (%zu triangulos)\n", path.c_str(),
                 g_triangles.size());
        } else {
          printf("Erro ao carregar: %s\n", path.c_str());
        }
      }
      return;
    }
    if (over_export) {
      // Exporta a malha atual como STL ASCII com a escala aplicada.
      if (!g_model_loaded) {
        printf("Nenhum modelo para exportar.\n");
        return;
      }
      std::string path = save_file_dialog();
      if (!path.empty()) {
        if (export_stl_ascii(path)) {
          printf("Modelo exportado: %s\n", path.c_str());
        } else {
          printf("Erro ao exportar.\n");
        }
      }
      return;
    }
    if (over_texture) {
      // Carrega uma textura; se ainda não há modelo, apenas avisa que ela
      // será usada quando um modelo for carregado.
      std::string path = open_texture_dialog();
      if (!path.empty()) {
        if (load_texture(path)) {
          if (!g_model_loaded) {
            printf("A textura sera exibida quando um modelo for carregado.\n");
          }
          glutPostRedisplay();
        }
      }
      return;
    }
    if (over_showcase) {
      // Liga/desliga o modo vitrine e reinicia o cronômetro.
      if (g_showcase_active) {
        g_showcase_active = false;
        g_showcase_time = 0.0f;
      } else {
        g_showcase_active = true;
        g_showcase_time = 0.0f;
      }
      printf("Showcase: %s\n", g_showcase_active ? "ATIVO" : "INATIVO");
      glutPostRedisplay();
      return;
    }

    // Clique fora dos botões: tenta selecionar uma lâmpada próxima ao cursor.
    int lamp_idx = pick_lamp(x, y);
    if (lamp_idx >= 0) {
      // Clique em lâmpada não selecionada a seleciona e inicia o arraste;
      // clique novamente nela a desseleciona.
      if (g_selected_lamp != lamp_idx) {
        g_selected_lamp = lamp_idx;
        g_dragging = true;
        g_last_mx = x;
        g_last_my = y;
      } else {
        g_selected_lamp = -1;
      }
      glutPostRedisplay();
      return;
    }
    if (g_selected_lamp >= 0) {
      // Clique fora de uma lâmpada selecionada a desseleciona.
      g_selected_lamp = -1;
      glutPostRedisplay();
    }

    // Caso contrário, inicia o arraste para rotacionar a câmera.
    g_dragging = true;
    g_last_mx = x;
    g_last_my = y;
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    // Soltar o botão encerra qualquer arraste em andamento.
    g_dragging = false;
  }

  // Roda do mouse: botão 3 (scroll para cima) aumenta o zoom, botão 4 reduz.
  if (button == 3) {
    g_zoom = std::min(g_zoom + 0.1f, 3.0f);
  }
  if (button == 4) {
    g_zoom = std::max(g_zoom - 0.1f, 0.3f);
  }
  glutPostRedisplay();
}

// Trata o movimento do mouse. Durante o arraste: arrasta a lâmpada selecionada
// sobre o plano do olhar ou rotaciona a câmera. Também atualiza o estado de
// hover dos botões (usado pelo motion e pelo passive motion).
static void motion(int x, int y) {
  if (g_dragging) {
    if (g_selected_lamp >= 0) {
      // Arrasta a lâmpada: converte a posição do mouse em um ponto 3D sobre o
      // plano perpendicular à câmera que passa pela lâmpada e desloca-a pelo
      // delta entre o ponto atual e o anterior.
      Vec3 view_dir = get_view_direction();
      int idx = g_selected_lamp;
      Vec3 curr = mouse_to_3d_plane(x, y, view_dir, g_lamp_positions[idx]);
      Vec3 prev = mouse_to_3d_plane(g_last_mx, g_last_my, view_dir,
                                    g_lamp_positions[idx]);
      g_lamp_positions[idx] = g_lamp_positions[idx] + (curr - prev);
      g_last_mx = x;
      g_last_my = y;
      glutPostRedisplay();
    } else if (!g_showcase_active) {
      // Rotaciona a câmera conforme o deslocamento do mouse.
      int dx = x - g_last_mx;
      int dy = y - g_last_my;
      g_rot_y += dx * 0.5f;
      g_rot_x += dy * 0.5f;
      g_last_mx = x;
      g_last_my = y;
      glutPostRedisplay();
    }
  }

  // Atualiza o hover dos botões conforme a posição do cursor.
  g_import_hover = (x >= 20 && x <= 140 && y >= 50 && y <= 85);
  g_export_hover = (x >= 150 && x <= 270 && y >= 50 && y <= 85);
  g_texture_hover = (x >= 280 && x <= 400 && y >= 50 && y <= 85);
  g_showcase_hover = (x >= 410 && x <= 530 && y >= 50 && y <= 85);
  glutPostRedisplay();
}

// Trata o teclado: atalhos equivalentes aos botões (i=importar, e=exportar,
// t=textura, s=showcase), ESC para sair, e as teclas de escala X/x Y/y Z/z
// tratadas pelo módulo de transformação.
static void keyboard(unsigned char key, int, int) {
  if (handle_scale_key(key))
    return;
  switch (key) {
  case 'i':
  case 'I': {
    // Importar modelo (mesmo fluxo do botão Importar).
    g_showcase_active = false;
    g_showcase_time = 0.0f;
    std::string path = open_file_dialog();
    if (!path.empty()) {
      if (load_stl(path)) {
        orient_model();
        center_model();
        generate_uv_coords();
        printf("Modelo carregado: %s (%zu triangulos)\n", path.c_str(),
               g_triangles.size());
      } else {
        printf("Erro ao carregar: %s\n", path.c_str());
      }
    }
    break;
  }
  case 'e':
  case 'E': {
    // Exportar modelo atual.
    if (!g_model_loaded) {
      printf("Nenhum modelo.\n");
      break;
    }
    std::string path = save_file_dialog();
    if (!path.empty()) {
      if (export_stl_ascii(path))
        printf("Exportado: %s\n", path.c_str());
      else
        printf("Erro ao exportar.\n");
    }
    break;
  }
  case 't':
  case 'T': {
    // Carregar textura.
    std::string path = open_texture_dialog();
    if (!path.empty()) {
      if (load_texture(path)) {
        if (!g_model_loaded) {
          printf("A textura sera exibida quando um modelo for carregado.\n");
        }
        glutPostRedisplay();
      }
    }
    break;
  }
  case 's':
  case 'S':
    // Alternar modo vitrine.
    if (g_showcase_active) {
      g_showcase_active = false;
      g_showcase_time = 0.0f;
    } else {
      g_showcase_active = true;
      g_showcase_time = 0.0f;
    }
    printf("Showcase: %s\n", g_showcase_active ? "ATIVO" : "INATIVO");
    glutPostRedisplay();
    break;
  case 27:
    exit(0); // ESC encerra a aplicação.
  }
}

// ---------------------------------------------------------------------------
// Inicialização e loop principal
// ---------------------------------------------------------------------------

int main(int argc, char **argv) {
  // Configura o GLUT e cria a janela com double buffering (cor + profundidade).
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(g_win_w, g_win_h);
  glutCreateWindow("Visualizador 3D STL - Computacao Grafica");

  // Estado inicial do OpenGL: teste de profundidade (ocultação de faces), auto
  // normalização de normais (importante para a escala não uniforme) e cor de
  // fundo da cena.
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glClearColor(0.15f, 0.15f, 0.2f, 1.0f);

  // Posições iniciais das duas luzes da cena.
  g_lamp_positions.push_back(Vec3(1.0f, 1.2f, 1.5f));
  g_lamp_positions.push_back(Vec3(-1.0f, -0.5f, 2.5f));

  // Registra os callbacks de desenho e interação.
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc(idle);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutPassiveMotionFunc(motion); // hover dos botões sem botão pressionado
  glutKeyboardFunc(keyboard);

  // Se um caminho de arquivo foi passado como argumento, carrega-o já no
  // início, aplicando o mesmo pós-processamento do carregamento manual.
  if (argc > 1) {
    if (load_stl(argv[1])) {
      orient_model();
      center_model();
      generate_uv_coords();
      printf("Modelo carregado: %s (%zu triangulos)\n", argv[1],
             g_triangles.size());
    }
  }

  glutMainLoop();
  return 0;
}
