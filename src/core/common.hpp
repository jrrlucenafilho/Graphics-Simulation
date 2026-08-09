#ifndef COMMON_HPP
#define COMMON_HPP

// Definições e declarações compartilhadas por todos os módulos do programa.
// Este cabeçalho concentra apenas o que é usado em mais de uma unidade de
// compilação: os tipos geométricos básicos e as variáveis globais da aplicação.

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <cmath>
#include <string>
#include <vector>

// Vetor 3D: representa posições, normais e deslocamentos no espaço. Os
// operadores e métodos (soma, subtração, produto por escalar, produto
// escalar, produto vetorial e normalização) simplificam a matemática vetorial
// usada na renderização, na orientação dos modelos e na interação com o mouse.
struct Vec3 {
  float x, y, z;
  Vec3() : x(0), y(0), z(0) {}
  Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
  Vec3 operator-(const Vec3 &o) const {
    return Vec3(x - o.x, y - o.y, z - o.z);
  }
  Vec3 operator+(const Vec3 &o) const {
    return Vec3(x + o.x, y + o.y, z + o.z);
  }
  Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
  Vec3 operator-() const { return Vec3(-x, -y, -z); }
  float dot(const Vec3 &o) const { return x * o.x + y * o.y + z * o.z; }
  Vec3 cross(const Vec3 &o) const {
    return Vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
  }
  float length() const { return sqrtf(x * x + y * y + z * z); }
  void normalize() {
    float l = length();
    if (l > 1e-8f) {
      x /= l;
      y /= l;
      z /= l;
    }
  }
};

// Vetor 2D de coordenadas de textura (u, v) de um vértice do triângulo.
struct Vec2 {
  float u, v;
  Vec2() : u(0), v(0) {}
  Vec2(float u, float v) : u(u), v(v) {}
};

// Triângulo da malha do modelo STL: normal da face (necessária para a
// iluminação), os três vértices e as coordenadas de textura (UV) de cada
// vértice (necessárias para aplicar uma textura sobre a superfície).
struct Triangle {
  Vec3 normal, v[3];
  Vec2 uv[3];
};

// ---------------------------------------------------------------------------
// Variáveis globais. Declaradas aqui com "extern" e definidas nos .cpp para
// que qualquer módulo possa acessar o estado central da aplicação.
// ---------------------------------------------------------------------------

// Estado do modelo carregado: malha de triângulos e rotação atual da câmera.
extern std::vector<Triangle> g_triangles;
extern bool g_model_loaded;
extern float g_rot_x, g_rot_y;

// Estado da interação com o mouse: última posição, se há arrasto em andamento,
// nível de zoom e tamanho atual da janela.
extern int g_last_mx, g_last_my;
extern bool g_dragging;
extern float g_zoom;
extern int g_win_w, g_win_h;

// Estado de destaque (hover) dos botões da interface.
extern bool g_import_hover;
extern bool g_export_hover;
extern bool g_texture_hover;

// Estado da textura carregada (identificador GL e caminho do arquivo de
// origem, usado para referenciá-la na exportação OBJ/MTL).
extern GLuint g_texture_id;
extern bool g_texture_loaded;
extern std::string g_texture_path;

// Estado do modo vitrine (Showcase): câmera percorre uma curva em volta do
// modelo automaticamente por alguns segundos.
extern bool g_showcase_hover;
extern bool g_showcase_active;
extern float g_showcase_time;
extern float g_showcase_duration;

// Posições das luzes pontuais da cena e qual delas está selecionada para ser
// arrastada com o mouse.
extern std::vector<Vec3> g_lamp_positions;
extern int g_selected_lamp;

// Transformações do modelo (translação, rotação e escala não uniforme)
// aplicadas na renderização via matriz de transformação; a geometria em
// memória não é alterada.
extern float g_translate_x, g_translate_y, g_translate_z;
extern float g_model_rot_x, g_model_rot_y, g_model_rot_z;
extern float g_scale_x, g_scale_y, g_scale_z;

#endif
