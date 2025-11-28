# picostack
A minimal stack-based programming language. It has one stack and 13 commands (so far) and I don't know how useful it is. It's my first time writing a project in C, so the code might be a bit goofy and not optimized. It's WIP, very buggy, and VERY prone to memory leaks (popping and pushing use ```realloc``` every time).
## Commands
```
p <num> : Push literal number onto the stack
a       : Pop & add top two numbers on the stack then push result back
s       : Pop & subtract top two numbers on the stack then push result back
r       : Reverse entire stack
j       : Jump to address @ top of stack if second-to top is 0 (pops both)
d       : Duplicate top number on the stack
w       : Swap top two numbers
o       : Output top number as character & pop
n       : Output top number as integer & pop
i       : Input some characters (max 64 bytes) and push them all*
u       : Input one number and push to the stack*
x       : Discard top number on the stack
c       : clear the stack

z       : (DEBUG) output the stack
reset   : (shell only) reset the session
 ```
 <small>\*Bug where inputting prints 2 `>`'s, idk why</small>
 ## Examples
### Hello, world!
```
p33p100p108p114p111p87p32p111p108p108p101p72oooooooooooo
```
or a more intuitive version:

### Infinite loop
```
p0p0j
```
### cat (with stack error)
```
irp0op0p4j
```
