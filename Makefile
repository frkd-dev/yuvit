OS = $(shell uname)
MAKEFILE = gnu

ifeq ($(OS),Darwin)
    MAKEFILE = osx
endif
ifneq (,$(findstring MINGW,$(OS)))
    MAKEFILE = mingw
endif

default:
	@echo "Running $(MAKEFILE) build..."
	@$(MAKE) -f Makefile.$(MAKEFILE) 

clean:
	@$(MAKE) -f Makefile.$(MAKEFILE) clean 

