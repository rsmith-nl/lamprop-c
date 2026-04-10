# file: Makefile
# vim:fileencoding=utf-8:fdm=marker:ft=make
#
# Copyright © 2018 R.F. Smith <rsmith@xs4all.nl>
# Created: 2018-05-15T01:05:44+0200
# Last modified: 2026-04-10T19:42:56+0200
.SUFFIXES:
.PHONY: clean

DOCSRC:= lamprop-manual.tex
DOCPDF:= ${DOCSRC:.tex=.pdf}
LMKARGS:=-bibtex -pdfxe -silent -file-line-error -halt-on-error

all:  ${DOCPDF}

${DOCPDF}: ${DOCSRC} lman.bib data/atlac430.tex data/help-output.txt
	-latexmk ${LMKARGS} ${DOCSRC} 2>&1

data/help-output.txt: ../lamprop
	../lamprop -h >data/help-output.txt

data/atlac430.tex: ../lamprop data/atlac430.lam
	../lamprop -le data/atlac430.lam >data/atlac430.tex

clean::
	latexmk -c >/dev/null 2>&1
	rm -f data/help-output.txt data/atlac430.tex
