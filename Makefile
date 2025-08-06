# Package name and version: BASENAME-VMAJOR.VMINOR.VPATCH.tar.gz
BASENAME = lamprop  ## Name for the project
VMAJOR   = 2025
VMINOR   = 08
VPATCH   = 03

# Define the C compiler to be used, if not cc.
#CC = gcc

# The next line is for building debugging libraries.
CFLAGS = -pipe -std=c11 -fPIC -g3 -Wall -Wextra -Wstrict-prototypes -Wpedantic \
                -Wshadow-all -Wmissing-field-initializers -Wpointer-arith \
                -fsanitize=address,undefined
LFLAGS += -s -pipe -fsanitize=address,undefined

# Other libraries to link against
LIBS += -lm

PREFIX = ${HOME}/.local  ## Root for the installation dicrectory tree.
BINDIR = $(PREFIX)/bin  ## Location where the binary will be installed.
MANDIR = $(PREFIX)/man/man1 ## Location for the manual-page.
DOCSDIR= $(PREFIX)/share/doc/$(BASENAME)  ## Location for the documentation

##### Maintainer stuff goes here:
DISTFILES = Makefile  ## Files that need to be included in the distribution.

# Source files.
SRCS = lamprop.c stringview.c parser.c arena.c  ## source code files.

##### No editing necessary beyond this point

all: $(BASENAME)  ## Compile the program. (default)

# This makefile uses a unit build.
$(BASENAME): $(SRCS) version.h
	$(CC) $(CFLAGS) $(LFLAGS) $(LDIRS) -o $(BASENAME) $(SRCS) $(LIBS)

.PHONY: clean
clean:  ## Remove all generated files.
	rm -f $(BASENAME) *~ core gmon.out $(TARFILE)* version.h backup-*

.PHONY: install
install: $(BASENAME)  ## Install the program.
	install -d $(BINDIR)
	install -m 755 $(BASENAME) $(BINDIR)
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

tags:  ## Update tags file
	uctags --language-force=C --kinds-C=+p-f *.h *.c

.PHONY: help
help:  ## List available commands
	@echo "make variables:"
	@echo
	@sed -n -e '/##/s/=.*\#\#/\t/p' Makefile
	@echo
	@echo "make targets:"
	@echo
	@sed -n -e '/##/s/:.*\#\#/\t/p' Makefile

# Predefined directory/file names
PKGDIR  = $(BASENAME)-$(VMAJOR).$(VMINOR).$(VPATCH)  ## Directory name in the package.
TARFILE = $(PKGDIR).tar.gz  ## Filename for the package.

dist: clean  # Build a tar distribution file
	rm -rf $(PKGDIR)
	mkdir -p $(PKGDIR)
	cp $(DISTFILES) $(XTRA_DIST) *.c *.h $(PKGDIR)
	tar -czf $(TARFILE) $(PKGDIR)
	rm -rf $(PKGDIR)
