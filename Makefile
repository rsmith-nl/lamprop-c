# vim:fileencoding=utf-8:ft=make

# Package name and version
BASENAME = lamprop
RELDATE=2026.08.25

# Define the C compiler to be used, if not cc.
#CC = gcc

# For debugging builds.
DCFLAGS = -pipe -std=c11 -g3 -Wall -Wextra -Wstrict-prototypes -Wpedantic \
                -Wshadow-all -Wmissing-field-initializers -Wpointer-arith \
                -fsanitize=address,undefined
DLFLAGS = -pipe -fsanitize=address,undefined

# For release builds.
CFLAGS = -Os -pipe -std=c11 -ffast-math -march=native
LFLAGS = -pipe -flto

# Other libraries to link against
LIBS += -lm

# Installation location for the current user
UPREFIX = ${HOME}/.local
UBINDIR = $(UPREFIX)/bin
UMANDIR = $(UPREFIX)/man/man1
UDOCSDIR= $(UPREFIX)/share/doc/$(BASENAME)

# System-wide installation location.
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/man/man1
DOCSDIR= $(PREFIX)/share/doc/$(BASENAME)

##### Maintainer stuff goes here:
DISTFILES = Makefile

# Source files.
SRCS = lamprop.c stringview.c sbuf.c parser.c arena.c setup.c core.c
SRCS += matrix.c text.c latex.c html.c utils.c logging.c

##### No editing necessary beyond this point

all: $(BASENAME)  ## Compile the program. (default)

debug: $(BASENAME)-debug ## Compile a debug version of the program.

# This makefile uses a unit build.
$(BASENAME): $(SRCS) version.h
	$(CC) $(CFLAGS) $(LFLAGS) $(LDIRS) -o $(BASENAME) $(SRCS) $(LIBS)

$(BASENAME)-debug: $(SRCS) version.h
	$(CC) $(DCFLAGS) $(DLFLAGS) $(LDIRS) -o $(BASENAME)-debug $(SRCS) $(LIBS)

.PHONY: clean
clean:  ## Remove all generated files.
	rm -f $(BASENAME) $(BASENAME)-debug *~ core gmon.out $(TARFILE) backup-*

install: $(BASENAME)  ## Install the program system-wide.
	install -d $(BINDIR)
	install -m 755 -s $(BASENAME) $(BINDIR)
#	install -m 644 $(BASENAME).1 $(MANDIR)
#	gzip -f -q $(MANDIR)/$(BASENAME).1

install-user: $(BASENAME)  ## Install the program for the current user.
	install -d $(UBINDIR)
	install -m 755 -s $(BASENAME) $(UBINDIR)
#	install -m 644 $(BASENAME).1 $(UMANDIR)
#	gzip -f -q $(UMANDIR)/$(BASENAME).1

.PHONY: uninstall
uninstall:  ## Uninstall the program.
	rm -f $(BINDIR)/$(BASENAME)

.PHONY: uninstall-user
uninstall-user:  ## Uninstall the program for the current user.
	rm -f $(UBINDIR)/$(BASENAME)

.PHONY: style
style:  ## Reformat source code using astyle.
	astyle -n --style=1tbs -s2 -p -o -O --indent-switches --delete-empty-lines --add-braces *.c *.h

.PHONY: tidy
tidy:  ## Run static code checker clang-tidy.
	clang-tidy19 --use-color --quiet *.c *.h --

.PHONY: man
man:  ## Show the rendered manual page
	mandoc -Tutf8 $(BASENAME).1 | less

tags: $(SRCS) *.h  ## Update tags file
	uctags --language-force=C --kinds-C=+p-f *.h *.c

.PHONY: test
test: $(BASENAME)  ## Run a test
	./lamprop test/hyer.lam

.PHONY: rel
rel: $(BASENAME).exe  ## Build a binary release for windows platform
	mkdir releases/$(BASENAME)-c-w64-$(RELDATE)
	cp $(BASENAME).exe README.rst doc/lamprop-manual.pdf releases/$(BASENAME)-c-w64-$(RELDATE)/
	rm -f releases/*.zip
	cd releases/ && zip -qr $(BASENAME)-c-w64-$(RELDATE).zip $(BASENAME)-c-w64-$(RELDATE)/

.PHONY: help
help:  ## List available commands
	@echo "make targets:"
	@echo
	@sed -n -e '/##/s/:.*\#\#/\t/p' Makefile

# Predefined directory/file names
PKGDIR  = $(BASENAME)-$(RELDATE)
TARFILE = $(BASENAME)-$(RELDATE).tar.gz

dist: clean  ## Build a source distribution tar file
	rm -rf $(PKGDIR)
	mkdir -p $(PKGDIR)
	cp $(DISTFILES) $(XTRA_DIST) *.c *.h $(PKGDIR)
	tar -czf $(TARFILE) $(PKGDIR)
	rm -rf $(PKGDIR)
