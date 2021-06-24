The LC-21i Assembler and Simulator
===============

To aid in testing your processor, we have provided an *assembler* and
a *simulator* for the LC-21i architecture. The assembler supports
converting text `.s` files into either binary (for the simulator) or
hexadecimal (for pasting into CircuitSim) formats.

Requirements
-----------

The assembler and simulator run on any version of Python 2.6+. An
instruction set architecture definition file is required along with
the assembler. The LC-21i assembler definition is included.

Included Files
-----------

* `assembler.py`: the main assembler program
* `lc21i.py`: the LC-21i assembler definition

Using the Assembler
-----------

### Assemble and Simulate

Example usage to assemble `test.s`:

    python assembler.py -i lc21i --sym test.s

... the resulting `test.bin` and `test.sym` files can then be loaded
in the simulator (see below).

### Assemble for CircuitSim

To output assembled code in hexadecimal (for use with *CircuitSim*):

    python assembler.py -i lc21i --hex test.s

You can then open the resulting `test.hex` file in your favorite text
editor.  In CircuitSim, right-click on your RAM, select **Edit
Contents...**, and then copy-and-paste the contents of the hex file
into the window.

Do not use the Open or Save buttons in CircuitSim, as it will not
recognize the format.

Simulator
-----------

We will not provide simulators for the `lc21i` instruction set.
However, you may continue to use the simulator from project 1 for
programs that do not use any `lc21i` specific instructions.

Assembler Pseudo-ops
-----------

In addition to the syntax described in the LC-21 ISA reference,
the assembler supports the following *pseudo-instructions*:

* `.fill`: fills a word of memory with the literal value provided

For example, `mynumber: .fill 0x500` places `0x500` at the memory
location labeled by `mynumber`.

* `.word`: an alias for `.fill`
