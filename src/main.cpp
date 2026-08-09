#include "core/common.hpp"
#include "core/transform.hpp"
#include "io/file_dialog.hpp"
#include "interaction/interaction.hpp"
#include "render/scene.hpp"
#include "io/stl_io.hpp"
#include "io/texture.hpp"
#include <cstdio>
#include <cstdlib>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

std::vector<Triangle> g_triangles;
bool g_model_loaded = false;
float g_rot_x = 0.0f, g_rot_y = 0.0f;

int g_last_mx = 0, g_last_my = 0;
bool g_dragging = false;
float g_zoom = 1.0f;
int g_win_w = WINDOW_WIDTH, g_win_h = WINDOW_HEIGHT;

bool g_import_hover = false;
bool g_export_hover = false;
bool g_texture_hover = false;

GLuint g_texture_id = 0;
bool g_texture_loaded = false;

bool g_paint_mode = false;
bool g_show_palette = false;
Vec3 g_selected_color(1.0f, 0.0f, 0.0f);
bool g_paint_hover = false;
bool g_has_painted_faces = false;

bool g_showcase_hover = false;
bool g_showcase_active = false;
float g_showcase_time = 0.0f;
float g_showcase_duration = 8.0f;

std::vector<Vec3> g_lamp_positions;
int g_selected_lamp = -1;

static void display() { render_scene(); }

static void reshape(int w, int h)
{
  g_win_w = w;
  g_win_h = h;
  glViewport(0, 0, w, h);
}

static void idle()
{
  if (g_showcase_active)
  {
    g_showcase_time += 0.016f;
    if (g_showcase_time >= g_showcase_duration)
    {
      g_showcase_active = false;
      g_showcase_time = 0.0f;
    }
    glutPostRedisplay();
  }
}

static void mouse(int button, int state, int x, int y)
{
  bool over_import = (x >= 20 && x <= 140 && y >= 50 && y <= 85);
  bool over_export = (x >= 150 && x <= 270 && y >= 50 && y <= 85);
  bool over_texture = (x >= 280 && x <= 400 && y >= 50 && y <= 85);
  bool over_paint = (x >= 410 && x <= 530 && y >= 50 && y <= 85);
  bool over_showcase = (x >= 540 && x <= 660 && y >= 50 && y <= 85);

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    if (over_paint)
    {
      g_showcase_active = false;
      g_showcase_time = 0.0f;
      if (g_paint_mode || g_show_palette)
      {
        g_paint_mode = false;
        g_show_palette = false;
      }
      else
      {
        g_show_palette = true;
      }
      printf("Modo pintura: %s\n", g_paint_mode ? "ATIVO" : "INATIVO");
      glutPostRedisplay();
      return;
    }

    if (g_show_palette)
    {
      int ss = 50, sp = 20;
      int total_w = 3 * ss + 2 * sp;
      int start_x = (g_win_w - total_w) / 2;
      int pal_y = 100;
      int sy = pal_y + 15;
      if (y >= sy && y <= sy + ss)
      {
        if (x >= start_x && x <= start_x + ss)
        {
          g_selected_color = Vec3(1, 0, 0);
          g_paint_mode = true;
          printf("Cor selecionada: VERMELHO\n");
          glutPostRedisplay();
          return;
        }
        if (x >= start_x + ss + sp && x <= start_x + 2 * ss + sp)
        {
          g_selected_color = Vec3(0, 1, 0);
          g_paint_mode = true;
          printf("Cor selecionada: VERDE\n");
          glutPostRedisplay();
          return;
        }
        if (x >= start_x + 2 * ss + 2 * sp && x <= start_x + 3 * ss + 2 * sp)
        {
          g_selected_color = Vec3(0, 0, 1);
          g_paint_mode = true;
          printf("Cor selecionada: AZUL\n");
          glutPostRedisplay();
          return;
        }
      }
    }

    if (over_import)
    {
      g_showcase_active = false;
      g_showcase_time = 0.0f;
      std::string path = open_file_dialog();
      if (!path.empty())
      {
        if (load_stl(path))
        {
          center_model();
          generate_uv_coords();
          reset_model_transform();
          g_paint_mode = false;
          g_show_palette = false;
          g_has_painted_faces = false;
          printf("Modelo carregado: %s (%zu triangulos)\n", path.c_str(),
                 g_triangles.size());
        }
        else
        {
          printf("Erro ao carregar: %s\n", path.c_str());
        }
      }
      return;
    }
    if (over_export)
    {
      if (!g_model_loaded)
      {
        printf("Nenhum modelo para exportar.\n");
        return;
      }
      std::string path = save_file_dialog();
      if (!path.empty())
      {
        if (export_stl_ascii(path))
        {
          printf("Modelo exportado: %s\n", path.c_str());
        }
        else
        {
          printf("Erro ao exportar.\n");
        }
      }
      return;
    }
    if (over_texture)
    {
      std::string path = open_texture_dialog();
      if (!path.empty())
      {
        if (load_texture(path))
        {
          if (!g_model_loaded)
          {
            printf("A textura sera exibida quando um modelo for carregado.\n");
          }
          glutPostRedisplay();
        }
      }
      return;
    }
    if (over_showcase)
    {
      if (g_showcase_active)
      {
        g_showcase_active = false;
        g_showcase_time = 0.0f;
      }
      else
      {
        g_showcase_active = true;
        g_showcase_time = 0.0f;
      }
      printf("Showcase: %s\n", g_showcase_active ? "ATIVO" : "INATIVO");
      glutPostRedisplay();
      return;
    }

    if (g_paint_mode && g_model_loaded)
    {
      g_dragging = true;
      g_last_mx = x;
      g_last_my = y;
      int idx = pick_triangle(x, y);
      if (idx >= 0)
      {
        g_triangles[idx].color = g_selected_color;
        g_has_painted_faces = true;
        glutPostRedisplay();
      }
      return;
    }

    int lamp_idx = pick_lamp(x, y);
    if (lamp_idx >= 0)
    {
      if (g_selected_lamp != lamp_idx)
      {
        g_selected_lamp = lamp_idx;
        g_dragging = true;
        g_last_mx = x;
        g_last_my = y;
      }
      else
      {
        g_selected_lamp = -1;
      }
      glutPostRedisplay();
      return;
    }
    if (g_selected_lamp >= 0)
    {
      g_selected_lamp = -1;
      glutPostRedisplay();
    }

    g_dragging = true;
    g_last_mx = x;
    g_last_my = y;
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
  {
    g_dragging = false;
  }

  if (button == 3)
  {
    g_zoom = std::min(g_zoom + 0.1f, 3.0f);
  }
  if (button == 4)
  {
    g_zoom = std::max(g_zoom - 0.1f, 0.3f);
  }
  glutPostRedisplay();
}

static void motion(int x, int y)
{
  if (g_dragging)
  {
    if (g_paint_mode && g_model_loaded && !g_showcase_active)
    {
      int idx = pick_triangle(x, y);
      if (idx >= 0)
      {
        g_triangles[idx].color = g_selected_color;
        g_has_painted_faces = true;
        glutPostRedisplay();
      }
    }
    else if (g_selected_lamp >= 0)
    {
      Vec3 view_dir = get_view_direction();
      int idx = g_selected_lamp;
      Vec3 curr =
          mouse_to_3d_plane(x, y, view_dir, g_lamp_positions[idx]);
      Vec3 prev = mouse_to_3d_plane(g_last_mx, g_last_my, view_dir,
                                    g_lamp_positions[idx]);
      g_lamp_positions[idx] = g_lamp_positions[idx] + (curr - prev);
      g_last_mx = x;
      g_last_my = y;
      glutPostRedisplay();
    }
    else if (!g_showcase_active)
    {
      int dx = x - g_last_mx;
      int dy = y - g_last_my;
      g_rot_y += dx * 0.5f;
      g_rot_x += dy * 0.5f;
      g_last_mx = x;
      g_last_my = y;
      glutPostRedisplay();
    }
  }

  g_import_hover = (x >= 20 && x <= 140 && y >= 50 && y <= 85);
  g_export_hover = (x >= 150 && x <= 270 && y >= 50 && y <= 85);
  g_texture_hover = (x >= 280 && x <= 400 && y >= 50 && y <= 85);
  g_paint_hover = (x >= 410 && x <= 530 && y >= 50 && y <= 85);
  g_showcase_hover = (x >= 540 && x <= 660 && y >= 50 && y <= 85);
  glutPostRedisplay();
}

static void special_keys(int key, int, int)
{
  handle_transform_special_key(key);
}

static void keyboard(unsigned char key, int, int)
{
  if (handle_transform_key(key))
    return;
  switch (key)
  {
  case 'i':
  case 'I':
  {
    g_showcase_active = false;
    g_showcase_time = 0.0f;
    std::string path = open_file_dialog();
    if (!path.empty())
    {
      if (load_stl(path))
      {
        center_model();
        generate_uv_coords();
        reset_model_transform();
        printf("Modelo carregado: %s (%zu triangulos)\n", path.c_str(),
               g_triangles.size());
      }
      else
      {
        printf("Erro ao carregar: %s\n", path.c_str());
      }
    }
    break;
  }
  case 'e':
  case 'E':
  {
    if (!g_model_loaded)
    {
      printf("Nenhum modelo.\n");
      break;
    }
    std::string path = save_file_dialog();
    if (!path.empty())
    {
      if (export_stl_ascii(path))
        printf("Exportado: %s\n", path.c_str());
      else
        printf("Erro ao exportar.\n");
    }
    break;
  }
  case 't':
  case 'T':
  {
    std::string path = open_texture_dialog();
    if (!path.empty())
    {
      if (load_texture(path))
      {
        if (!g_model_loaded)
        {
          printf("A textura sera exibida quando um modelo for carregado.\n");
        }
        glutPostRedisplay();
      }
    }
    break;
  }
  case 'p':
  case 'P':
    if (g_paint_mode || g_show_palette)
    {
      g_paint_mode = false;
      g_show_palette = false;
    }
    else
    {
      g_show_palette = true;
    }
    printf("Modo pintura: %s\n", g_paint_mode ? "ATIVO" : "INATIVO");
    glutPostRedisplay();
    break;
  case 's':
  case 'S':
    if (g_showcase_active)
    {
      g_showcase_active = false;
      g_showcase_time = 0.0f;
    }
    else
    {
      g_showcase_active = true;
      g_showcase_time = 0.0f;
    }
    printf("Showcase: %s\n", g_showcase_active ? "ATIVO" : "INATIVO");
    glutPostRedisplay();
    break;
  case 27:
    exit(0);
  }
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(g_win_w, g_win_h);
  glutCreateWindow("Visualizador 3D STL - Computacao Grafica");

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glClearColor(0.15f, 0.15f, 0.2f, 1.0f);

  g_lamp_positions.push_back(Vec3(1.0f, 1.2f, 1.5f));
  g_lamp_positions.push_back(Vec3(-1.0f, -0.5f, 2.5f));

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc(idle);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutPassiveMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special_keys);

  if (argc > 1)
  {
    if (load_stl(argv[1]))
    {
      center_model();
      generate_uv_coords();
      reset_model_transform();
      printf("Modelo carregado: %s (%zu triangulos)\n", argv[1],
             g_triangles.size());
    }
  }

  glutMainLoop();
  return 0;
}
