
# 🦖 Compilador HULK
Un compilador para el lenguaje HULK implementado en C++

# 📂 Estructura del Proyecto

```plaintext
.
├── build/          # Directorio generado(ejecutables y objetos)
├── include/        # Headers (.hpp)
│   ├── lexer.hpp   # Analizador léxico
│   ├── parser.hpp  # Analizador sintáctico
│   └── ast.hpp     # Árbol de sintaxis abstracta
├── src/            # Código fuente (.cpp)
│   ├── main.cpp    # Punto de entrada
│   ├── lexer.cpp   # Implementación del lexer
│   ├── parser.cpp  # Implementación del parser
│   └── ast.cpp     # Implementación del AST
├── script.hulk     # Ejemplo de código HULK (input)
└── Makefile        # Automatiza compilación y ejecución
```
## ⚙️ Dependencias
Compilador: g++ (C++17)

Herramientas: make

# 🚀 Compilación y Ejecución
## Compilar (genera ejecutable en build/hulkc)
make build

## Ejecutar el compilador con un archivo .hulk
make run  # Usa script.hulk por defecto

## Limpiar archivos generados
make clean

# 🔧 Componentes Clave
Lexer: Convierte código fuente en tokens.

Parser: Construye el AST a partir de tokens.

AST: Representación estructurada del código.

Generación de código: Transforma el AST a código objetivo (opcional).

## 📌 Notas
El ejecutable final es build/hulkc.

Modifica script.hulk para probar diferentes entradas.

## 📋 Ejemplo de Uso
Escribe código HULK en script.hulk:

print("¡Hola Mundo!");

### Ejecuta:

make build

make run