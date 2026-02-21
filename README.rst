=====================================================
Calculating elastic properties of composite laminates
=====================================================

.. Note:: This is the C version of this program. There is also a `Python version <https://github.com/rsmith-nl/lamprop/>`_

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

History
=======

The original version of ``lamprop`` was written in C around 2006.

In 2011, I ported the program to Python since I had become more proficient in
it than I was in C.

Around 2023 I came across the `blog <https://nullprogram.com/index/>`_ of Chris Wellons.
The techniques he shows for `arena allocation`_ and `string handling`_ *transformed*
C for me. Memory allocation and string handling are now almost effortless.

Furthermore, I got the request to make ``lamprop`` available for colleagues.
And the IT department at my employer does not want to support anything that
(like Python) requires a separate runtime.

So starting in 2025 I ported the Python version back to C (C11 to be more
specific), using the techniques I learned from Chris Wellons. That is what is
contained in this repository.

.. _arena allocation: https://nullprogram.com/blog/2023/09/27/
.. _string handling: https://nullprogram.com/blog/2023/01/18/#implementation-highlights
