# 🦖 HULK Compiler

This project is a compiler for the HULK programming language, developed as part of a third-year Computer Science course in Compiler Construction at university. The compiler is implemented in C and leverages Flex and Bison for lexical and syntactic analysis, and LLVM for code generation.

## Project Overview ℹ️

The HULK compiler translates source code written in the HULK language into LLVM IR and/or native executables. The project demonstrates the full compilation pipeline, including lexical analysis, parsing, AST construction, semantic analysis, and code generation.

This project is part of the curriculum for the "Compilación" (Compiler Construction) course, aiming to provide hands-on experience in building a working compiler from scratch.

## Project Structure 🏛️

- **build/**  
  Contains build artifacts, object files, and the final compiler executable.

- **src/**  
  Main source code for the compiler.
  - **main.c**: Entry point for the compiler.
  - **backend/**: Code generation logic.
    - **codegen/**: General code generation.
    - **llvm/**: LLVM-specific code generation.
  - **frontend/**: Frontend logic.
    - **ast/**: Abstract Syntax Tree node definitions and logic.
    - **hulk_type/**: Type system and type descriptors.
    - **scope/**: Symbol tables and scope management.
    - **semantic_check/**: Semantic analysis and type checking.
    - **parser/**: Parser definitions (Bison).
    - **lexer/**: Lexer definitions (Flex).

- **test/**  
  Test cases for the compiler, organized by feature (basic, conditional, functions, etc.).

- **grammars/**  
  Documentation for the HULK grammar.

- **script.hulk**  
  Example HULK source code (to be provided by the user).

- **Makefile**  
  Build and automation instructions.

- **README.md**  
  Project documentation (this file).

- **WORKFLOW.md**  
  Project planning and workflow notes.

## Usage

Follow these steps to use the HULK compiler:

1. **Clone the repository**
   ```sh
   git clone <repo-url>
   cd HULK_compiler
2. **Compilar el proyecto**
    ```sh
    make compile
    ```

3. **Ejecutar el programa compilado**
    ```sh
    make execute
    ```