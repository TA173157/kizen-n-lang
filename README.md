# kizen-n-lang
# ⚙️ Kizen-N: A Custom LLVM Compiler

**Kizen-N** is a strongly-typed, object-oriented programming language built entirely from scratch. It features a custom Lexer, a recursive descent Parser, a strict Semantic Analyzer, and an LLVM-based IR Generator that compiles `.kzn` scripts directly into native machine code executables.

## 🚀 Key Features

* **Full Compiler Pipeline:** Raw text to Native Machine Code (`.exe` / `.out`).
* **Strongly Typed & Type-Safe:** Implements strict Semantic Analysis with an intelligent "VIP List" for safe implicit type promotions (e.g., automatically upcasting `int` to `double`).
* **Complex Memory Layouts:** Supports custom structs (`blueprint`) with precise memory alignment and `InBoundsGEP` pointer arithmetic.
* **Hardware-Level Math:** Full support for both the ALU (integers) and the FPU (32-bit floats and 64-bit doubles) using safe `FPExt` and `FPTrunc` casting.
* **Control Flow:** Turing-complete branching with `if/else`, `while`, and `for` loops.
* **Native I/O:** C-standard `printf` and `scanf` integration for runtime interactions.

## 💻 Syntax Example

Kizen-N uses a clean, TypeScript-inspired syntax. Here is an example of calculating the Intersection over Union (IoU) of two bounding boxes:
```typescript
blueprint Box {
    let x: float = 0.0f;
    let y: float = 0.0f;
    let width: float = 0.0f;
    let height: float = 0.0f;
}

function main(): int {
    let face1: Box = new Box();
    face1.width = 4.0f;
    face1.height = 4.0f;
    
    let face2: Box = new Box();
    face2.x = 2.0f;
    face2.width = 4.0f;
    
    // ... complex bounding box logic
    
    return 0;
}
