#include "file/parser.hpp"
#include "render/baseshader.hpp"
#include "render/renderbatch.hpp"
#include "render/renderbox.hpp"
#include "render/renderlist.hpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <cassert>

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

__time_t last_modified = 0;

u32 window_width, window_height;

static constexpr u32 START_WINDOW_WIDTH = 1240;
static constexpr u32 START_WINDOW_HEIGHT = 1754;

void batch_from_file(RenderBatch &batch, char const *filename) {
  batch.vertices.clear();
  batch.indices.clear();

  Parser p;

  CV cv = p.read_cv_file(filename);
  cv.width = window_width;
  cv.height = window_height;

  auto root_renderbox = RenderBox(cv.layout[0].root, cv);

  RenderList list;

  root_renderbox.render(0, 0, window_width, window_height, list);

  for (auto const &c : list) {
    batch.rect(c);
  }
  batch.end();
}

void loop(SDL_Window *w, char const *filename) {
  BaseShader shader;
  RenderBatch batch;

  batch_from_file(batch, filename);

  shader.use();
  glUniform2f(0, window_width, window_height);

  batch.use();

  bool is_running = true;
  while (is_running) {
    SDL_Event e;
    while (SDL_WaitEventTimeout(&e, 100)) {
      switch (e.type) {
      case SDL_EVENT_QUIT:
        is_running = false;
        return;
      case SDL_EVENT_WINDOW_RESIZED:
        window_width = e.window.data1;
        window_height = e.window.data2;
        glViewport(0, 0, window_width, window_height);
        glUniform2f(0, window_width, window_height);
        batch_from_file(batch, filename);
        goto draw;
      case SDL_EVENT_KEY_DOWN:
        if (e.key.scancode == SDL_SCANCODE_W) {
          SDL_SetWindowSize(w, START_WINDOW_WIDTH, START_WINDOW_HEIGHT);
        }
        break;
      }
    }
  draw:
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    batch.render();
    SDL_GL_SwapWindow(w);

    {
      struct stat result;
      if (stat(filename, &result) == 0) {
        if (last_modified < result.st_mtime) {
          last_modified = result.st_mtime;
          batch_from_file(batch, filename);
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 1;

  window_width = START_WINDOW_WIDTH;
  window_height = START_WINDOW_HEIGHT;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window =
      SDL_CreateWindow("cv", window_width, window_height,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

  SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, true);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetSwapInterval(1);

  auto ctx = SDL_GL_CreateContext(window);
  glewInit();
  glEnable(GL_BLEND);
  glBlendEquation(GL_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  {
    struct stat result;
    if (stat(argv[1], &result) == 0) {
      last_modified = result.st_mtime;
    } else {
      return 1;
    }
  }

  loop(window, argv[1]);

  SDL_GL_DestroyContext(ctx);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
