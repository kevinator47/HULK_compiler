# Planificación Proyecto

## Fase I

### Alcance 
Definir Lenguaje (Versión Reducida)

### Tareas: 
- [] Definir tokens
- [] Definir gramática

### Versión Reducida de HULK
La primera versión de HULK incluye:
- Literales: number, string, bool, constantes
- Expresiones Aritméticas :  + - * / ** ^ ()
- Expresiones de Comparación: < <= > >= == !=
- Expresiones Lógicas : ! | &
- Expresiones Concatenación: @ @@
- Expresiones Condicionales
- Declaración de variables
- Declaración y llamado de funciones (soporte Recursividad)
- Funciones Predefinidas 
- Bloques de expresiones

## Fase II

### Alcance  
Implementar compilador apoyándose en bibliotecas(Flex, Bison, etc)

### Tareas
- [] Definir regex para cada token
- [] Implementar Lexer (usando Flex)
- [] Definir reglas de produccion
- [] Implementar Parser (usando Bison)
- [] Crear clases/nodos del AST
- [] Implementar patron visitor(Arbol derivacion -> AST )
- [] Analisis semantico: Validaciones , inferencia de tipos ?
- [] Usar API LLVM (AST -> LLVM)

## Fase III

### Alcance
Sustituir implementaciones de bibliotecas por implementaciones propias

### Tareas
- [] Implementar motor regex
- [] Implementar generador de parsers
- [] Implementar generador LLVM

### Fase IV

### Alcance 
Extender el lenguaje HULK para abarcar toda la implementacion

Version Completa de HULK
- Version Reducida
- Iteradores
- Bucles
- Vectores
- Declaracion de Tipos
- Protocolos
- Functores

### Tareas
Por definir