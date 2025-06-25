# C-Compiler

Self-compiling compiler in C. Because why not.

## Introduction

This project documents my journey to write a self-compiling compiler.
A compiler that can compile its own source code is often considered a "real" compiler. Therefore, the ultimate goal of this project is to create a compiler that can compile itself. Additionally, I aim to target real hardware platforms, specifically the ARM64 architecture used by the M1 Mac.
Throughout this journey, I will share my progress, code, and learnings step by step. This will not only help me clarify my thoughts but also provide a resource for others interested in compiler development.

P.S. I heavily used repository [acwj](https://github.com/DoctorWkt/acwj) by DoctorWkt.

## Goals of the Project

Here are the goals I aim to achieve with this project:

1. Write a Self-Compiling Compiler: The compiler should be able to compile its own source code.
2. Target Real Hardware Platforms: The compiler’s output should run on real hardware. For this project, I will target the ARM64 architecture of the M1 Mac.
3. Practical Over Theoretical: I will focus on a practical approach, starting from scratch and introducing theory only when necessary.
4. Follow the KISS Principle: "Keep it simple, stupid!"

## Target Language

For this project, I’ve chosen a subset of the C programming language as the target language. C is close to assembly language, which makes it easier to translate C code into assembly. Additionally, I enjoy working with C, and it’s a great choice for this type of project.
The target language will include the basic features of C but exclude complex structures like standard libraries. This will simplify the development process and allow me to focus on the core functionality of the compiler.

##  Development Environment

The project will be developed on an M1 Mac and will target the ARM64 architecture. Below are the tools and environment I’ll use:

- macOS: The native operating system of the M1 Mac.
- Xcode and Command Line Tools: To provide essential development tools.
- Homebrew: For installing additional tools.
- LLVM/Clang: For generating and testing ARM64 assembly code.

[Click here to move to the next step.](https://github.com/mertgrsy/C-Compiler/00_DevelopmentEnviroment/Readme.md)
