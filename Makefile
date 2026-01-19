# Package name and version: BASENAME-VMAJOR.VMINOR.VPATCH.tar.gz
BASENAME = lamprop
VMAJOR   = 2026
VMINOR   = 01
VPATCH   = 20

# Define the C compiler to be used, if not cc.
#CC = gcc

# The next lines are for building debugging libraries.
#CFLAGS = -pipe -std=c11 -fPIC -g3 -Wall -Wextra -Wstrict-prototypes -Wpedantic \
#                -Wshadow-all -Wmissing-field-initializers -Wpointer-arith \
#                -fsanitize=address,undefined
#LFLAGS = -pipe -fsanitize=address,undefined

# The next lines are for release builds.
CFLAGS = -Os -pipe -std=c11 -fPIC -ffast-math -march=native
CFLAGS += -DNDEBUG=1
LFLAGS = -pipe -flto


# Other libraries to link against
LIBS += -lm

PREFIX = ${HOME}/.local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/man/man1
DOCSDIR= $(PREFIX)/share/doc/$(BASENAME)

##### Maintainer stuff goes here:
DISTFILES = Makefile

# Source files.
SRCS = lamprop.c stringview.c parser.c arena.c setup.c core.c
SRCS += matrix.c text.c latex.c

##### No editing necessary beyond this point

all: $(BASENAME)  ## Compile the program. (default)

# This makefile uses a unit build.
$(BASENAME): $(SRCS) version.h
	$(CC) $(CFLAGS) $(LFLAGS) $(LDIRS) -o $(BASENAME) $(SRCS) $(LIBS)

.PHONY: clean
clean:  ## Remove all generated files.
	rm -f $(BASENAME) *~ core gmon.out $(TARFILE) version.h backup-*

.PHONY: install
install: $(BASENAME)  ## Install the program.
	install -d $(BINDIR)
	install -m 755 -s $(BASENAME) $(BINDIR)
	install -m 644 $(BASENAME).1 $(MANDIR)
	gzip -f -q $(MANDIR)/$(BASENAME).1

.PHONY: uninstall
uninstall:  ## Uninstall the program.
	rm -f $(BINDIR)/$(BASENAME)

version.h:
	echo '#define VERSION "'${VMAJOR}"."${VMINOR}"."${VPATCH}'"' >version.h

.PHONY: style
style:  ## Reformat source code using astyle.
	astyle -n *.c *.h

.PHONY: tidy
tidy:  ## Run static code checker clang-tidy.
	clang-tidy19 --use-color --quiet *.c *.h --

.PHONY: man
man:  ## Show the rendered manual page
	mandoc -Tutf8 $(BASENAME).1 | less

tags: $(SRCS) *.h  ## Update tags file
	uctags --language-force=C --kinds-C=+p-f *.h *.c

.PHONY: help
help:  ## List available commands
	@echo "make targets:"
	@echo
	@sed -n -e '/##/s/:.*\#\#/\t/p' Makefile

# Predefined directory/file names
PKGDIR  = $(BASENAME)-$(VMAJOR).$(VMINOR).$(VPATCH)
TARFILE = $(BASENAME)-$(VMAJOR).$(VMINOR).$(VPATCH).tar.gz

dist: clean  # Build a tar distribution file
	rm -rf $(PKGDIR)
	mkdir -p $(PKGDIR)
	cp $(DISTFILES) $(XTRA_DIST) *.c *.h $(PKGDIR)
	tar -czf $(TARFILE) $(PKGDIR)
	rm -rf $(PKGDIR)
