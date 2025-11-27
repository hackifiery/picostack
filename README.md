# picostack
A minimal stack-based programming language. It has one stack and 10 commands (so far) and I don't know how useful it is. It's my first time writing a project in C, so the code might be a bit goofy and not optimized. It's WIP, very buggy, and VERY prone to memory leaks (popping and pushing use ```realloc``` every time).
## Commands
```
p <num> : Push literal number onto the stack
a       : Add top two numbers on the stack
s       : Subtract top two numbers on the stack
r       : Reverse entire stack
j       : Jump to address @ top of stack if second-to top is 0 (pops both)
d       : Duplicate top number on the stack
w       : Swap top two numbers
o       : Output top number as character
i       : Input some characters (max 64 bytes) and push them all*
x       : Discard top number on the stack
 ```
 <small>\*Bug where inputting prints 2 `>`'s, idk why</small>
 ## Examples
### Hello, world!
```
p33p100p108p114p111p87p32p111p108p108p101p72oooooooooooo
```
or a more intuitive version:
```
# Push characters for "Hello World!" onto the stack (in reverse order)

p33   # '!' character
p100  # 'd'
p108  # 'l'
p114  # 'r'
p111  # 'o'
p87   # 'W'
p32   # ' ' (space)
p111  # 'o'
p108  # 'l'
p108  # 'l'
p101  # 'e'
p72   # 'H'

# Output all characters
o  # output 'H'
o  # output 'e'
o  # output 'l'
o  # output 'l'
o  # output 'o'
o  # output ' ' (space)
o  # output 'W'
o  # output 'o'
o  # output 'r'
o  # output 'l'
o  # output 'd'
o  # output '!'
```
### Infinite loop
```
p0p0j
```
### cat (with stack error)
```
irp0op4p0j
```
or
```
i  # input stuff
r  # reverse cuz stacks are FILO
p0 # null character so nothing is printed the first time through
# begin loop
o  # output char
p4 # jump addr
p0 # unconditional jump
j  # will eventually terminate on an error...
```
