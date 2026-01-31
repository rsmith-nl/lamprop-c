=====================================================
Calculating elastic properties of composite laminates
=====================================================

The purpose of this program is to calculate some properties of
fiber-reinforced composite laminates. It calculates
- engineering properties like Ex, Ey, Gxy
- thermal properties CTE_x and CTE_y
- physical properties like density and laminate thickness
- stiffness and compliance matrices (ABD and abd)

This program can _not_ calculate the strength of composite laminates; because
there are many different failure modes, strengths of composite laminates
cannot readily be calculated from the strengths of the separate materials that
form the laminate. These strengths have to be determined from tests.

The programs works by reading plain text input files that contain one or more laminate
definitions written in a simple domain specific language.

The program has options for producing LaTeX and HTML output in addition to
plain text output.

The program and its file format are documented by a manual. This can be found
in the ``doc`` subdirectory.

Writing a program such as this requires domain expertise and judgement.
Therefore “AI” / LLM-generated contributions and rewrites are not welcome.


Requirements
============

To build this program you will need a C compiler that supports the C11
standard.
The build has been tested with clang 19 and gcc 15.

POSIX
-----

On a POSIX system, the program can be built by simply invoking ``make``.

ms-windows
----------

The only tested build method on ms-windows is using w64devkit_.
Open ``w64devkit``, change to the directory where you have stored the source,
and issue the command ``make -f Makefile.win32``.

.. _w64devkit: https://github.com/skeeto/w64devkit


Installation
============

The program does not *require* installation to run. Only the binary is
required.

For convenience you can copy the binary to a location in your ``$PATH``.

POSIX
-----

On a POSIX system, ``make install`` will install the program in ``~/.local/bin``.

Vim
---

In the ``tools`` subdirectory you will find a vim_ syntax file for lamprop
files. If you want to use it, copy ``lamprop.vim`` to ``~/.vim/syntax``, and
set the filetype of your lamprop files to ``lamprop``.

.. _vim: http://www.vim.org

You can set the filetype by adding a modeline to your lamprop files:

.. code-block:: vim

    vim:ft=lamprop

This requires that modeline support is enabled. You should have the following
line in your ``vimrc``:

.. code-block:: vim

    set modeline

Alternatively, if you use the ``.lam`` extension for your lamprop files you
can use an autocommand in your ``vimrc``;

.. code-block:: vim

    autocmd BufNewFile,BufRead *.lam set filetype=lamprop

