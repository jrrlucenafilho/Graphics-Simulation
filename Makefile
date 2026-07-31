CXX      = g++
CXXFLAGS = -Isrc
LDFLAGS  = -lGL -lGLU -lglut
SRC      = src
BLD      = build

_OBJS    = main.o \
           core/transform.o \
           io/stl_io.o io/texture.o io/file_dialog.o \
           render/scene.o render/ui.o \
           interaction/interaction.o
OBJS     = $(patsubst %,$(BLD)/%,$(_OBJS))

$(BLD)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stl_viewer: $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BLD) stl_viewer
