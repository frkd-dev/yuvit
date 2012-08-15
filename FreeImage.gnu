# Linux makefile for FreeImage

include FreeImage.srcs

LIBS = -lstdc++

OBJDIR_32 = _linux/32
OBJDIR_64 = _linux/64

OBJS := $(SRCS:.c=.c.o)
OBJS := $(OBJS:.cpp=.cpp.o)
OBJDIRS := $(addprefix $(OBJDIR_32)/,$(SRCDIRS)) $(addprefix $(OBJDIR_64)/,$(SRCDIRS))
OBJS_32 = $(addprefix $(OBJDIR_32)/,$(OBJS))
OBJS_64 = $(addprefix $(OBJDIR_64)/,$(OBJS))
CFLAGS = -O3 -fPIC -fexceptions -fvisibility=hidden -DNO_LCMS $(INCDIRS)
CXXFLAGS = $(CFLAGS) -Wno-ctor-dtor-privacy 

TARGET = freeimage
STATICLIB_32 = $(OBJDIR_32)/lib$(TARGET).a
STATICLIB_64 = $(OBJDIR_64)/lib$(TARGET).a
HEADER = FreeImage/Source/FreeImage.h

default: FreeImage

dirs:
	@mkdir -p $(OBJDIRS)

FreeImage: dirs $(STATICLIB_32) $(STATICLIB_64)
	cp FreeImage/Source/FreeImage.h FreeImage/Dist

$(OBJDIR_32)/%.c.o: %.c
	$(CC) $(CFLAGS) -m32 -c $< -o $@

$(OBJDIR_64)/%.c.o: %.c
	$(CC) $(CFLAGS) -m64 -c $< -o $@

$(OBJDIR_32)/%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -m32 -c $< -o $@

$(OBJDIR_64)/%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -m64 -c $< -o $@

$(STATICLIB_32): $(OBJS_32)
	$(AR) r $@ $(OBJS_32)

$(STATICLIB_64): $(OBJS_64)
	$(AR) r $@ $(OBJS_64)

clean:
	rm -f $(OBJS_32) $(OBJS_64) $(STATICLIB_32) $(STATICLIB_64)

