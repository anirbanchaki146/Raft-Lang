# Raft
This was supposed to be a programming language that I was workng on when I was 15 years old. While, I had intended it to use LLVM, it currently operates through an interpreter.

## Syntax
1. Immutability is enforced. Values are declared using let keyword. To make a variable (mutable), add the var keyword after let (Like Rust's mut). 
```
let a = 10; // This is immutable by default
let var b = 20; // This is a variable / mutable
```
2. Raft is statically typed. Each variable has a fixed type which cannot be changed. Type annotations can be added but are not necessary; Raft's type checker can infer the type at runtime.
```
let a = 10.34; // Without type annotations (Raft automatically infers this as a double)
let b: int = 20; // With type annotations

let c: double = 20; // Valid
let d: int = 20.5 // Would throw an error, integers can be internally typecasted to doubles but not vice versa as it may cause data loss
```
3. If statements (Condition need not be within parentheses):
```
if condition {
    statements
}
```
4. While loops (Condition need not be within parentheses):
```
while condition {
    statements
}
```
5. Functions:
```
fn sum(a: int, b: int) -> int {
    return a + b;
}
```
6. Print stuff using println()
```
println("Hello, world"); 
```
7. String concatenation is supported.
```
println("Hello, " + "World!");
```

## Limitations
This is a solo project and bugs may inadvertently creep in. Further, due to academic pressures, I will not be able to work on Raft for a substantial amount of time. Updates and bug fixes will be slow. Currently, println calls are directly resolved by the interpreter and there is no Foreign Function Interface. In the future (when the academic pressure is off), I intend to migrate this project to LLVM.

## Instructions
### Direct Executable
Pre-compiled binaries for Windows and linux are available at https://github.com/anirbanchaki146/Raft-Lang/releases

### To build from source (For testing and/or debugging):
1. Download and install CMake from https://cmake.org/download/ (If you haven't already). 
2. Download the repository and in a terminal window, enter and run : `cmake [path to downloaded repository]`.
3. CMake will generate the build files. Once completed, enter: `cmake --build [path to downloaded repository]`
4. The Raft interpreter is produced at `[repo directory]/bin`
5. Run without any arguments for the JIT interpreter or pass a file location as argument.
6. A sample file test.rft is provided for testing.
7. If you find any bugs, report them so that Raft can be improved for everyone else.
