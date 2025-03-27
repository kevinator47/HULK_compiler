# Gramática de HULK [parcial]

A continuación se presenta la gramática actual del lenguaje HULK, que incluye expresiones aritmeticas, condicionales, definiciones y llamado de funciones, reasignaciones, bloques de expresiones y más.

---

## Gramática LL(1)

```plaintext
S       → Program

Program → FunctionDef Program
         | P OpEnd

P       → Exp
         | Block

Exp    → IfExp
        | LetIn
        | LogicOr

Block → { ExpList }

ExpList → P ; ExpList
         | ε

## Condicionales

IfExp  → if ( Exp ) P OpEnd ElifList else P OpEnd

ElifList → elif ( Exp ) P OpEnd ElifList
          | ε

## Declaración de variables

LetIn   → let AssignList in P OpEnd

AssignList → id = Exp MoreAssign
           | ε

MoreAssign → , id = Exp MoreAssign
            | ε

## Operaciones Lógicas y Aritméticas
LogicOr → LogicAnd OpOr
OpOr    → | LogicAnd OpOr
         | ε

LogicAnd → Compare OpAnd
OpAnd    → & Compare OpAnd
          | ε

Compare → S OpCompare
OpCompare → == X OpCompare
          | != X OpCompare
          | < X OpCompare
          | > X OpCompare
          | <= X OpCompare
          | >= X OpCompare
          | ε

X      → F Op1
Op1    → + X Op1
       | - X Op1
       | @ X Op1
       | @@ X Op1
       | ε

F      → T Op3
Op3    → ** T Op3
       | ^ T Op3
       | ε

## Términos
T      → num
       | id TRest
       | pred_func (ArgListCall)
       | string
       | ( Exp )
       | - T
       | PI
       | E
       | true
       | false
       | ! T

TRest  → ( ArgListCall )    // llamado a función
       | := Exp             // reasignación
       | ε                  // variable

## Definición de funciones
FunctionDef → function id ( ArgList ) => Exp ;
            | function id ( ArgList ) Block

ArgList → id MoreArgs
         | ε

MoreArgs → , id MoreArgs
          | ε

## Llamado de Funciones
ArgListCall → Exp MoreArgsCall
            | ε

MoreArgsCall → , Exp MoreArgsCall
             | ε