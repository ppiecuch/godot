# 03 — Control Flow

Demonstrates how branching, loops, and pattern matching translate to C.

## if / elif / else → JUMP / JUMPIF / JUMPIFNOT

```gdscript
if x > 0:
    return "positive"
elif x < 0:
    return "negative"
else:
    return "zero"
```

Becomes a chain of compare + conditional jump instructions:

```
Block_0:
    OPERATOR(GT, x, 0) -> _tmp0
    JUMPIFNOT _tmp0 -> Block_1     # skip to elif
    RETURN "positive"

Block_1:                            # elif
    OPERATOR(LT, x, 0) -> _tmp1
    JUMPIFNOT _tmp1 -> Block_2     # skip to else
    RETURN "negative"

Block_2:                            # else
    RETURN "zero"
```

## for-in → ITERATEBEGIN / ITERATE

```gdscript
for item in items:
    total += item
```

Translates to a two-instruction loop pattern:

```
Block_header:
    ITERATEBEGIN(items, counter, item) -> Block_body, Block_exit

Block_body:
    OPERATOR(ADD, total, item) -> total
    ITERATE(items, counter, item) -> Block_body, Block_exit

Block_exit:
    ...
```

The `ITERATEBEGIN` initializes the iterator and jumps to exit if empty.
`ITERATE` advances and jumps to exit when exhausted.

## while → JUMP + JUMPIFNOT

```gdscript
while i < n:
    sum += i
    i += 1
```

```
Block_test:
    OPERATOR(LT, i, n) -> _cond
    JUMPIFNOT _cond -> Block_exit

Block_body:
    OPERATOR(ADD, sum, i) -> sum
    OPERATOR(ADD, i, 1) -> i
    JUMP -> Block_test

Block_exit:
    ...
```

## match → sequential JUMPIFNOT chain

`match` compiles to the same pattern as chained if/elif — there is no
dedicated match opcode in GDScript bytecode.

## break / continue

- `break` → `JUMP` to the loop's exit block
- `continue` → `JUMP` to the loop's header/test block
