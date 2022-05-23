## questions

1. 

2.

3.

4. One interesting thing to note is what happens when a child exits;
what happens to its children in the process tree? To study this, let’s
create a specific example: ./fork.py -A a+b,b+c,c+d,c+e,c-.
This example has process ’a’ create ’b’, which in turn creates ’c’,
which then creates ’d’ and ’e’. However, then, ’c’ exits. What do
you think the process tree should like after the exit? What if you
use the -R flag? Learn more about what happens to orphaned pro-
cesses on your own to add more context.

When process 'c' exits, its child processes become children of the top
level process (their parent process is now the first in the tree). The 
-R option assigns the parent of the orphaned process to the next available
ancestor. 

The process' parent is set to 'init' (process ID 1) if it is orphaned.

5.

6. Finally, use both -t and -F together. This shows the final process
tree, but then asks you to fill in the actions that took place. By look-
ing at the tree, can you determine the exact actions that took place?
In which cases can you tell? In which can’t you tell? Try some dif-
ferent random seeds to delve into this question

It is difficult to tell the order of operations, and whether a process was 
forked from another process but was orphaned and it's parent set to the top 
level process (or another parent).

```
Action?
Action?
Action?
Action?
Action?

                        Final Process Tree:
                               a
                               ├── c
                               │   └── d
                               └── e
```

a forks b
a forks c (or b forks c before exiting)
b exits (somewhere?)
c forks d
a forks e

```
Action?
Action?
Action?
Action?
Action?

                        Final Process Tree:
                               a
                               └── d
```

a forks b
a forks c
a forks d
b exits (somewhere?)
c exits (somewhere?)


