# Raft
This was supposed to be a programming language that I was workng on when I was 15 years old. While, I had intended it to use LLVM, it currently operates through an interpreter.

## Syntax
1. Immutability is enforced. Make a variable using:
```
let a = 10; // This is immutable by default
let var b = 20; // This is a variable / mutable
```
2. If statments:
```
if condition {
    statements
}
```
3. While loops:
```
while condition {
    statements
}
```

## Instructions:
1. Generate makefiles using cmake
2. Build executable using make
3. A sample file test.rft is provided for testing
4. Run without any arguments for JIT or pass a file location as argument