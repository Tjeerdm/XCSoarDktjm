# Build rules for the portable screen library

SCREEN_SRC_DIR = $(SRC)/Screen

SCREEN_SOURCES = \
	$(SCREEN_SRC_DIR)/Debug.cpp \
	$(SCREEN_SRC_DIR)/ProgressBar.cpp \
	$(SCREEN_SRC_DIR)/Util.cpp \
	$(SCREEN_SRC_DIR)/Icon.cpp \
	$(SCREEN_SRC_DIR)/Brush.cpp \
	$(SCREEN_SRC_DIR)/Canvas.cpp \
	$(SCREEN_SRC_DIR)/Color.cpp \
	$(SCREEN_SRC_DIR)/BufferCanvas.cpp \
	$(SCREEN_SRC_DIR)/Pen.cpp \
	$(SCREEN_SRC_DIR)/Window.cpp \
	$(SCREEN_SRC_DIR)/BufferWindow.cpp \
	$(SCREEN_SRC_DIR)/DoubleBufferWindow.cpp \
	$(SCREEN_SRC_DIR)/SingleWindow.cpp

SCREEN_CUSTOM_SOURCES = \
	$(SCREEN_SRC_DIR)/Custom/Timer.cpp \
	$(SCREEN_SRC_DIR)/Custom/TextWindow.cpp \
	$(SCREEN_SRC_DIR)/Custom/LargeTextWindow.cpp \
	$(SCREEN_SRC_DIR)/Custom/ButtonWindow.cpp \
	$(SCREEN_SRC_DIR)/Custom/Window.cpp \
	$(SCREEN_SRC_DIR)/Custom/WList.cpp \
	$(SCREEN_SRC_DIR)/Custom/ContainerWindow.cpp \
	$(SCREEN_SRC_DIR)/Custom/CheckBox.cpp \
	$(SCREEN_SRC_DIR)/Custom/EditWindow.cpp \
	$(SCREEN_SRC_DIR)/Custom/TopWindow.cpp \
	$(SCREEN_SRC_DIR)/Custom/SingleWindow.cpp \
	$(SCREEN_SRC_DIR)/Custom/Canvas.cpp

ifeq ($(TARGET),ANDROID)
SCREEN_SOURCES += \
	$(SCREEN_CUSTOM_SOURCES) \
	$(SCREEN_SRC_DIR)/OpenGL/EGL.cpp \
	$(SCREEN_SRC_DIR)/Android/Window.cpp \
	$(SCREEN_SRC_DIR)/Android/TopWindow.cpp \
	$(SCREEN_SRC_DIR)/Android/SingleWindow.cpp \
	$(SCREEN_SRC_DIR)/Android/TopCanvas.cpp \
	$(SCREEN_SRC_DIR)/Android/Bitmap.cpp \
	$(SCREEN_SRC_DIR)/Android/Font.cpp
endif

ifeq ($(FREETYPE),y)
SCREEN_SOURCES += \
	$(SCREEN_SRC_DIR)/FreeType/Font.cpp \
	$(SCREEN_SRC_DIR)/FreeType/Init.cpp
endif

ifeq ($(LIBPNG),y)
SCREEN_SOURCES += \
	$(SCREEN_SRC_DIR)/OpenGL/LibPNG.cpp
endif

ifeq ($(LIBJPEG),y)
SCREEN_SOURCES += \
	$(SCREEN_SRC_DIR)/OpenGL/LibJPEG.cpp
endif

ifeq ($(OPENGL),y)
SCREEN_SOURCES += \
	$(SCREEN_SRC_DIR)/OpenGL/Init.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Globals.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Extension.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/FBO.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/VertexArray.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Bitmap.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/RawBitmap.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Cache.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Canvas.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/BufferCanvas.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/TopCanvas.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Texture.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Buffer.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Shapes.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Surface.cpp \
	$(SCREEN_SRC_DIR)/OpenGL/Triangulate.cpp
endif

ifeq ($(ENABLE_SDL),y)
SCREEN_SOURCES += $(SCREEN_CUSTOM_SOURCES)
SCREEN_SOURCES += \
	$(SCREEN_SRC_DIR)/Custom/Files.cpp \
	$(SCREEN_SRC_DIR)/SDL/Window.cpp \
	$(SCREEN_SRC_DIR)/SDL/TopWindow.cpp \
	$(SCREEN_SRC_DIR)/SDL/SingleWindow.cpp \
	$(SCREEN_SRC_DIR)/SDL/TopCanvas.cpp \
	$(SCREEN_SRC_DIR)/SDL/Bitmap.cpp \
	$(SCREEN_SRC_DIR)/SDL/Init.cpp
ifeq ($(FREETYPE),n)
SCREEN_SOURCES += \
	$(SCREEN_SRC_DIR)/SDL/Font.cpp
endif
ifeq ($(OPENGL),n)
SCREEN_SOURCES += \
	$(SCREEN_SRC_DIR)/SDL/Canvas.cpp \
	$(SCREEN_SRC_DIR)/SDL/Bitmap.cpp \
	$(SCREEN_SRC_DIR)/SDL/RawBitmap.cpp \
	$(SCREEN_SRC_DIR)/VirtualCanvas.cpp \
	$(SCREEN_SRC_DIR)/WindowCanvas.cpp
endif
else ifeq ($(EGL),y)
SCREEN_SOURCES += \
	$(SCREEN_CUSTOM_SOURCES) \
	$(SCREEN_SRC_DIR)/Custom/Files.cpp \
	$(SCREEN_SRC_DIR)/EGL/Init.cpp \
	$(SCREEN_SRC_DIR)/EGL/TopCanvas.cpp \
	$(SCREEN_SRC_DIR)/EGL/Window.cpp \
	$(SCREEN_SRC_DIR)/EGL/TopWindow.cpp \
	$(SCREEN_SRC_DIR)/EGL/SingleWindow.cpp
else ifeq ($(HAVE_WIN32),y)
SCREEN_SOURCES += \
	$(SCREEN_SRC_DIR)/VirtualCanvas.cpp \
	$(SCREEN_SRC_DIR)/WindowCanvas.cpp \
	$(SCREEN_SRC_DIR)/GDI/Init.cpp \
	$(SCREEN_SRC_DIR)/GDI/Font.cpp \
	$(SCREEN_SRC_DIR)/GDI/AlphaBlend.cpp \
	$(SCREEN_SRC_DIR)/GDI/Timer.cpp \
	$(SCREEN_SRC_DIR)/GDI/Window.cpp \
	$(SCREEN_SRC_DIR)/GDI/PaintWindow.cpp \
	$(SCREEN_SRC_DIR)/GDI/ContainerWindow.cpp \
	$(SCREEN_SRC_DIR)/GDI/TextWindow.cpp \
	$(SCREEN_SRC_DIR)/GDI/LargeTextWindow.cpp \
	$(SCREEN_SRC_DIR)/GDI/ButtonWindow.cpp \
	$(SCREEN_SRC_DIR)/GDI/EditWindow.cpp \
	$(SCREEN_SRC_DIR)/GDI/SingleWindow.cpp \
	$(SCREEN_SRC_DIR)/GDI/TopWindow.cpp \
	$(SCREEN_SRC_DIR)/GDI/Bitmap.cpp \
	$(SCREEN_SRC_DIR)/GDI/RawBitmap.cpp \
	$(SCREEN_SRC_DIR)/GDI/Canvas.cpp \
	$(SCREEN_SRC_DIR)/GDI/BufferCanvas.cpp \
	$(SCREEN_SRC_DIR)/GDI/PaintCanvas.cpp
GDI_CPPFLAGS = -DUSE_GDI

ifeq ($(HAVE_CE),y)
GDI_LDLIBS = -lcommctrl
else
GDI_LDLIBS = -lcomctl32 -luser32 -lgdi32 -lmsimg32
endif

ifeq ($(TARGET),PC)
GDI_LDLIBS += -Wl,-subsystem,windows
endif
endif

SCREEN_CPPFLAGS = $(SDL_CPPFLAGS) $(GDI_CPPFLAGS) $(OPENGL_CPPFLAGS) $(FREETYPE_CPPFLAGS) $(LIBPNG_CPPFLAGS) $(LIBJPEG_CPPFLAGS) $(EGL_CPPFLAGS)
SCREEN_LDLIBS = $(SDL_LDLIBS) $(GDI_LDLIBS) $(OPENGL_LDLIBS) $(FREETYPE_LDLIBS) $(LIBPNG_LDLIBS) $(LIBJPEG_LDLIBS) $(EGL_LDLIBS)

$(eval $(call link-library,screen,SCREEN))

SCREEN_LDADD += $(SDL_LDADD)
