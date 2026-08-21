
# C-GASM Compiler

C-GASM is a custom C compiler that translates C source code into **GASM (Ghost Assembly Language)**.

GASM is a custom stack-based assembly language and virtual machine developed as a separate project. C-GASM provides a higher-level programming interface for that architecture by taking C source code through a compiler pipeline and generating GASM assembly.

> **Status:** Experimental / actively evolving

---

## Overview

C-GASM implements a compiler pipeline that takes C source code and converts it into GASM assembly.

```text
             C Source Code
                   |
                   v
              +---------+
              |  Lexer  |
              +----+----+
                   |
                   v
              +---------+
              | Parser  |
              +----+----+
                   |
                   v
        +---------------------+
        | Semantic Analysis   |
        +----------+----------+
                   |
                   v
              +---------+
              | Codegen |
              +----+----+
                   |
                   v
             GASM Assembly
                (.gasm)
                   |
                   v
            GASM Assembler
                   |
                   v
             GASM Binary
                   |
                   v
                GASM VM

The compiler acts as the front-end connecting the C programming language to the GASM virtual machine.


---

Architecture

The compiler is divided into four main stages:

C Source
   |
   v
+----------------+
|    Lexer       |
+-------+--------+
        |
        v
+----------------+
|    Parser      |
+-------+--------+
        |
        v
+------------------------+
| Semantic Analysis      |
+-----------+------------+
            |
            v
+------------------------+
| Code Generation        |
+-----------+------------+
            |
            v
      GASM Assembly

Each stage has a separate implementation.


---

Compiler Pipeline

1. Lexical Analysis

The lexer reads C source code and converts it into a stream of tokens.

It identifies the lexical elements required by the compiler, including:

Keywords

Identifiers

Numeric literals

Character literals

String literals

Operators

Punctuation

Other language tokens


Implementation:

lexer.c
lexer.h


---

2. Parsing

The parser consumes the token stream produced by the lexer and determines the syntactic structure of the C program.

It processes expressions, statements, declarations, and other supported language constructs.

Implementation:

parser.c
parser.h


---

3. Semantic Analysis

The semantic analysis stage operates after parsing.

It checks semantic properties of the program and performs the analysis required before code generation.

Implementation:

sema.c
sema.h


---

4. Code Generation

The code generator converts the compiler's internal representation into GASM assembly.

This is the compiler backend.

C Program
    |
    v
Compiler Representation
    |
    v
GASM Instructions

Implementation:

codegen.c
codegen.h


---

Output

The compiler generates GASM source code.

C Source
   |
   v
C-GASM Compiler
   |
   v
output.gasm

The resulting .gasm file can then be assembled by the GASM assembler.


---

Complete Toolchain

C-GASM is designed to work together with the GASM project.

The complete toolchain is:

+-------------+
|  C Program  |
+------+------+
       |
       v
+------------------+
| C-GASM Compiler  |
+------+-----------+
       |
       v
+------------------+
|   GASM Assembly  |
|     (.gasm)      |
+------+-----------+
       |
       v
+------------------+
|  GASM Assembler  |
+------+-----------+
       |
       v
+------------------+
|   GASM Binary    |
+------+-----------+
       |
       v
+------------------+
|     GASM VM      |
+------+-----------+
       |
       v
+------------------+
| Program Execution|
+------------------+

In short:

C
 |
 v
C-GASM Compiler
 |
 v
GASM
 |
 v
GASM Assembler
 |
 v
GASM Binary
 |
 v
GASM VM

The two repositories form parts of the same larger system:

C-GASM Compiler provides the C-to-GASM compiler.

GASM provides the target assembly language, assembler, binary format, and virtual machine.



---

Project Structure

C-GASM-Compiler/
|
+-- lexer.c
+-- lexer.h
|
+-- parser.c
+-- parser.h
|
+-- sema.c
+-- sema.h
|
+-- codegen.c
+-- codegen.h
|
+-- main.c
|
+-- test.c
+-- output.gasm

Components

File	Purpose

lexer.c	Lexical analysis implementation
lexer.h	Lexer definitions
parser.c	Parser implementation
parser.h	Parser definitions
sema.c	Semantic analysis implementation
sema.h	Semantic analysis definitions
codegen.c	GASM code generation
codegen.h	Code generation definitions
main.c	Compiler entry point
test.c	Compiler test program
output.gasm	Example generated GASM output



---

Design

C-GASM is implemented as a small standalone compiler rather than relying on an existing compiler backend.

The main compiler components are implemented directly in C:

Lexer
  |
  v
Parser
  |
  v
Semantic Analysis
  |
  v
Code Generator

The generated code targets the custom GASM architecture instead of an existing CPU instruction set such as x86 or ARM.


---

Target Architecture

C-GASM targets:

GASM — Ghost Assembly Language

GASM is a custom stack-based assembly language and virtual machine.

C-GASM does not directly generate machine code for x86, ARM, or another conventional processor architecture.

Instead:

C
|
v
C-GASM Compiler
|
v
GASM Assembly
|
v
GASM Assembler
|
v
GASM Binary
|
v
GASM VM

The GASM virtual machine provides the execution environment for the generated programs.


---

Why GASM?

Using GASM as the compiler target allows the project to explore the complete path from a high-level programming language to program execution.

High-Level Language
        |
        v
     Compiler
        |
        v
   Assembly Language
        |
        v
     Assembler
        |
        v
      Binary
        |
        v
   Virtual Machine

Instead of targeting an existing processor architecture, the compiler targets an architecture designed specifically for the project.

This makes it possible to experiment with both compiler construction and virtual machine design as parts of the same system.


---

Example

A simple C program can be compiled into GASM assembly:

int main() {
    int a = 10;
    int b = 20;

    return a + b;
}

The compiler translates the source program into GASM instructions representing the same computation.

The generated assembly can then be assembled and executed by the GASM virtual machine.


---

Testing

The repository contains a test program:

test.c

The generated output is represented by:

output.gasm

This makes it possible to inspect the generated GASM assembly and verify the compiler output.


---

Building

The compiler is implemented in C.

A typical build using GCC is:

gcc main.c lexer.c parser.c sema.c codegen.c -o c-gasm

Depending on the current development environment, the build command may need to be adjusted.


---

Usage

The compiler takes C source code and produces GASM assembly.

Conceptually:

C source
   |
   v
c-gasm
   |
   v
GASM source

The generated GASM source can then be passed to the GASM assembler.

For example:

test.c
   |
   v
C-GASM
   |
   v
output.gasm
   |
   v
GASM Assembler
   |
   v
output.bin
   |
   v
GASM VM


---

Current Scope

C-GASM is an experimental compiler and does not implement the complete C language.

The compiler is being developed incrementally, with language features being added as the compiler and GASM runtime evolve.

It should therefore be considered a subset implementation of C, rather than a fully conforming C compiler.


---

Limitations

Current limitations include:

Not all of the C language is supported

No complete C standard library

Limited compiler tooling

Limited diagnostics compared with mature C compilers

Generated programs depend on the GASM runtime environment

GASM itself is an experimental target architecture



---

Future Work

Potential future improvements include:

Expanding C language support

Improving semantic analysis

Improving compiler diagnostics

Expanding type handling

Expanding expression support

Improving generated GASM code

Adding optimization passes

Developing a standard runtime/library for GASM

Improving interoperability with the GASM VM

Supporting larger C programs



---

Project Goals

The main goal of C-GASM is to explore compiler construction while targeting a completely custom execution environment.

The project provides practical experience with:

Lexical analysis

Parsing

Semantic analysis

Code generation

Instruction-set targeting

Runtime design

Virtual machine execution


Rather than targeting an existing processor, the compiler targets an architecture designed specifically for the project.


---

Relationship with GASM

C-GASM and GASM are two parts of the same larger project.

C-GASM
            Compiler Frontend
                   |
                   v
             GASM Assembly
                   |
                   v
                GASM
          Assembler + VM

The responsibilities are separated between the two projects:

C-GASM Compiler

Responsible for:

Reading C source code

Lexical analysis

Parsing

Semantic analysis

Generating GASM assembly


GASM

Responsible for:

Defining the instruction set

Assembling GASM source

Producing executable binaries

Providing the virtual machine

Executing the generated program

Providing runtime facilities



---

Related Project

GASM — Ghost Assembly Language

Custom stack-based assembly language, assembler, and virtual machine targeted by C-GASM.

Repository:

https://github.com/Pnatehc13/GASM-Ghost-Assembly-Language

