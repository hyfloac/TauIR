# Virtual Instruction

[TOC]

## Compiled Assembly

Instructions are encoded as a opcode and optional operands. Opcodes are typically a single byte, if the first byte has the high bit set, then it is a two byte instruction. Operands are encoded in little endian.

There is an execution stack, containing 32/64 bit values used for some instructions and to pass parameters to functions. There is the local function scope containing the storage for all local variables.

### Compiled Opcodes

| Opcode                | Opcode Encoding | Description                                                  | Operand 0                | Operand 1       | Operand 2      |
| --------------------- | --------------- | ------------------------------------------------------------ | ------------------------ | --------------- | -------------- |
| `Nop`                 | `0x00`          | A no-op.                                                     |                          |                 |                |
| `Push.0`              | `0x10`          | Push Local Value #0 onto the stack.                          |                          |                 |                |
| `Push.1`              | `0x11`          | Push Local Value #1 onto the stack.                          |                          |                 |                |
| `Push.2`              | `0x12`          | Push Local Value #2 onto the stack.                          |                          |                 |                |
| `Push.3`              | `0x13`          | Push Local Value #3 onto the stack.                          |                          |                 |                |
| `Push.N`              | `0x9010`        | Push Local Value #N onto the stack.                          | N `<u16>`                |                 |                |
| `Push.Arg.0`          | `0x30`          | Push 8 bytes from Argument #0 onto the stack.                |                          |                 |                |
| `Push.Arg.1`          | `0x31`          | Push 8 bytes from Argument #1 onto the stack.                |                          |                 |                |
| `Push.Arg.2`          | `0x32`          | Push 8 bytes from Argument #2 onto the stack.                |                          |                 |                |
| `Push.Arg.3`          | `0x33`          | Push 8 bytes from Argument #3 onto the stack.                |                          |                 |                |
| `Push.Arg.N`          | `0x9030`        | Push 8 bytes from Argument #`N` onto the stack.              | N `<u8>`                 |                 |                |
| `Push.Ptr`            | `0x9011`        | Push the data Pointed to by Local Value #`N` onto the stack. | N `<u16>`                |                 |                |
| `Push.Global`         | `0x9012`        | Push Global Value #`N` onto the stack.                       | N `<u32>`                |                 |                |
| `Push.Global.Ext`     | `0x9013`        | Push Global Value #`N` onto the stack, from an external module, specified by `Module`, the Local Value # containing the handle for the external module. | N `<u32>`                | Module `<u16>`  |                |
| `Push.Global.Ptr`     | `0x9014`        | Push the data Pointed to by Global Value #`N` onto the stack. | N `<u32>`                |                 |                |
| `Push.Global.Ext.Ptr` | `0x9015`        | Push the data Pointed to by Global Value #`N` onto the stack, from an external module, specified by `Module`, the Local Value # containing the handle for the external module. | N `<u32>`                | Module `<u16>`  |                |
| `Pop.0`               | `0x20`          | Pop into Local Value #0 from the stack.                      |                          |                 |                |
| `Pop.1`               | `0x21`          | Pop into Local Value #1 from the stack.                      |                          |                 |                |
| `Pop.2`               | `0x22`          | Pop into Local Value #2 from the stack.                      |                          |                 |                |
| `Pop.3`               | `0x23`          | Pop into Local Value #3 from the stack.                      |                          |                 |                |
| `Pop.N`               | `0xA020`        | Pop into Local Value #`N` from the stack.                    | N `<u16>`                |                 |                |
| `Pop.Arg.0`           | `0x40`          | Pop 8 bytes into Argument #0 from the stack.                 |                          |                 |                |
| `Pop.Arg.1`           | `0x41`          | Pop 8 bytes into Argument #1 from the stack.                 |                          |                 |                |
| `Pop.Arg.2`           | `0x42`          | Pop 8 bytes into Argument #2 from the stack.                 |                          |                 |                |
| `Pop.Arg.3`           | `0x43`          | Pop 8 bytes into Argument #3 from the stack.                 |                          |                 |                |
| `Pop.Arg.N`           | `0xA040`        | Pop 8 bytes into Argument #`N` from the stack.               | N `<u8>`                 |                 |                |
| `Pop.Ptr`             | `0xA021`        | Pop data from the stack and store it into the address pointed to by Local Value #`N`. | N `<u16>`                |                 |                |
| `Pop.Global`          | `0xA022`        | Pop into Global Value #`N` from the stack.                   | N `<u32>`                |                 |                |
| `Pop.Global.Ext`      | `0xA023`        | Pop into Global Value #`N` from the stack, in an external module, specified by `Module`, the local Value # containing the handle for the external module. | N `<u32>`                | Module `<u16>`  |                |
| `Pop.Global.Ptr`      | `0xA024`        | Pop data from the stack and store it into the address Pointed to by Global Value #`N`. | N `<u32>`                |                 |                |
| `Pop.Global.Ext.Ptr`  | `0xA025`        | Pop data from the stack and store it into the address Pointed to by Global Value #`N`, from an external module, specified by` Module`, the Local Value # containing the handle for the external module. | N `<u32>`                | Module `<u16>   |                |
| `Pop.Count`           | `0xA026`        | Pop `N` bytes from the stack.                                | N `<u16>`                |                 |                |
| `Dup.1`               | `0x0c`          | Duplicate top byte on the stack.                             |                          |                 |                |
| `Dup.2`               | `0x0D`          | Duplicate top 2 bytes on the stack.                          |                          |                 |                |
| `Dup.4`               | `0x0E`          | Duplicate top 4 bytes on the stack.                          |                          |                 |                |
| `Dup.8`               | `0x0F`          | Duplicate top 8 bytes on the stack.                          |                          |                 |                |
| `Expand.SX.1.2`       | `0x24`          | Pop 1 byte from the stack, sign extend it to 2 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.SX.1.4`       | `0x25`          | Pop 1 byte from the stack, sign extend it to 4 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.SX.1.8`       | `0x26`          | Pop 1 byte from the stack, sign extend it to 8 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.SX.2.4`       | `0x27`          | Pop 2 bytes from the stack, sign extend it to 4 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.SX.2.8`       | `0x28`          | Pop 2 bytes from the stack, sign extend it to 8 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.SX.4.8`       | `0x29`          | Pop 4 bytes from the stack, sign extend it to 8 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.ZX.1.2`       | `0x44`          | Pop 1 byte from the stack, zero extend it to 2 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.ZX.1.4`       | `0x45`          | Pop 1 byte from the stack, zero extend it to 4 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.ZX.1.8`       | `0x46`          | Pop 1 byte from the stack, zero extend it to 8 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.ZX.2.4`       | `0x47`          | Pop 2 bytes from the stack, zero extend it to 4 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.ZX.2.8`       | `0x48`          | Pop 2 bytes from the stack, zero extend it to 8 bytes, and push it back onto the stack. |                          |                 |                |
| `Expand.ZX.4.8`       | `0x49`          | Pop 4 bytes from the stack, zero extend it to 8 bytes, and push it back onto the stack. |                          |                 |                |
| `Trunc.8.4`           | `0x2A`          | Pop 8 bytes from the stack, truncate the upper 4 bytes, and push it back onto the stack as 4 bytes. |                          |                 |                |
| `Trunc.8.2`           | `0x2B`          | Pop 8 bytes from the stack, truncate the upper 6 bytes, and push it back onto the stack as 2 bytes. |                          |                 |                |
| `Trunc.8.1`           | `0x2C`          | Pop 8 bytes from the stack, truncate the upper 7 bytes, and push it back onto the stack as 1 byte. |                          |                 |                |
| `Trunc.4.2`           | `0x2D`          | Pop 4 bytes from the stack, truncate the upper 2 bytes, and push it back onto the stack as 2 bytes. |                          |                 |                |
| `Trunc.4.1`           | `0x2E`          | Pop 4 bytes from the stack, truncate the upper 3 bytes, and push it back onto the stack as 1 byte. |                          |                 |                |
| `Trunc.2.1`           | `0x2F`          | Pop 2 bytes from the stack, truncate the upper byte, and push it back onto the stack as 1 byte. |                          |                 |                |
| `Load`                | `0x1B`          | Load from memory address stored in Local Value #`Address`, into Local Value #`N`. | N `<u16>`                | Address `<u16>` |                |
| `Load.Global`         | `0x901A`        | Load from memory address stored in Local Value #`Address`, into Global Value #`N`. | N `<u32>`                | Address `<u16>` |                |
| `Load.Global.Ext`     | `0x901B`        | Load from memory address stored in Local Value #`Address`, into Global Value #`N` within external module, specified by the handle stored in Local Value #`Module`. | N `<u32>`                | Address `<u16>` | Module `<u16>` |
| `Store`               | `0x4A`          | Store into memory address stored in Local Value #`Address`, from Local Value #`N`. | Address `<u16>`          | N `<u16>`       |                |
| `Store.Global`        | `0xA04A`        | Store into memory address stored in Local Value #`Address`, from Global Value #`N`. | Address  `<u16>`         | N `<u32>`       |                |
| `Store.Global.Ext`    | `0xA04B`        | Store into memory address stored in Local Value #`Address`, from Global Value #`N` within external module, specified by the handle stored in Local Value #`Module`. | Address  `<u16>`         | N `<u32>`       | Module `<u16>` |
| `Const.0`             | `0x14`          | Push constant `0` as 4 byte.                                 |                          |                 |                |
| `Const.1`             | `0x15`          | Push constant `1` as 4 bytes.                                |                          |                 |                |
| `Const.2`             | `0x16`          | Push constant `2` as 4 bytes.                                |                          |                 |                |
| `Const.3`             | `0x17`          | Push constant `3` as 4 bytes.                                |                          |                 |                |
| `Const.4`             | `0x18`          | Push constant `4` as 4 bytes.                                |                          |                 |                |
| `Const.FF`            | `0x19`          | Push constant `0xFFFFFFFF` as 4 bytes.                       |                          |                 |                |
| `Const.7F`            | `0x1A`          | Push constant `0x7FFFFFFF` as 4 bytes.                       |                          |                 |                |
| `Const.N`             | `0x8B00`        | Push constant `C` as 4 bytes.                                | C `<u32>`                |                 |                |
| `Add.i32`             | `0x34`          | Pop 4 bytes into register `A`,  Pop 4 bytes into register `B`, Add `B` to `A` as an integer and push the 4 byte result onto the stack. |                          |                 |                |
| `Add.i64`             | `0x35`          | Pop 8 bytes into register `A`,  Pop 8 bytes into register `B`, Add `B` to `A` as an integer and push the 8 byte result onto the stack. |                          |                 |                |
| `Sub.i32`             | `0x36`          | Pop 4 bytes into register `A`,  Pop 4 bytes into register `B`, Subtract `B` from `A` as an integer and push the 4 byte result onto the stack. |                          |                 |                |
| `Sub.i64`             | `0x37`          | Pop 8 bytes into register `A`,  Pop 8 bytes into register `B`, Subtract `B` from `A` as an integer and push the 8 byte result onto the stack. |                          |                 |                |
| `Mul.i32`             | `0x38`          | Pop 4 bytes into register `A`,  Pop 4 bytes into register `B`, Multiply `A` by `B` as an integer and push the 4 byte result onto the stack. |                          |                 |                |
| `Mul.i64`             | `0x39`          | Pop 8 bytes into register `A`,  Pop 8 bytes into register `B`, Multiply `A` by `B` as an integer and push the 8 byte result onto the stack. |                          |                 |                |
| `Div.i32`             | `0x3A`          | Pop 4 bytes into register `A`,  Pop 4 bytes into register `B`, Divide `A` by `B` as an integer and push the 4 byte quotient onto the stack, followed by the 4 byte remainder onto the stack. If register `A` contains `23` and register `B` contains `5`, then `4` followed by `3` would be pushed onto the stack. |                          |                 |                |
| `Div.i64`             | `0x3B`          | Pop 8 bytes into register `A`,  Pop 8 bytes into register `B`, Divide `A` by `B` as an integer and push the 8 byte quotient onto the stack, followed by the 8 byte remainder onto the call stack. If register `A` contains `23` and register `B` contains `5`, then `4` followed by `3` would be pushed onto the stack. |                          |                 |                |
| `Comp.i32.Cond`       | `0x807X`        | Pop 4 bytes into register `A`,  Pop 4 bytes into register `B`, Compare `A` to `B` as an integer using the specified condition, and push the a 1 byte Boolean (0 or 1) onto the stack depending on whether the condition passed. If register `A` contains `1` and register `B` contains `2`, the comparison Greater (0x5) would result in a `0` being pushed onto the stack. |                          |                 |                |
| `Comp.i64.Cond`       | `0x808X`        | Pop 8 bytes into register `A`,  Pop 8 bytes into register `B`, Compare `A` to `B` as an integer using the specified condition, and push the a 1 byte Boolean (0 or 1) onto the stack depending on whether the condition passed. If register `A` contains `1` and register `B` contains `2`, the comparison Greater (0x5) would result in a `0` being pushed onto the stack. |                          |                 |                |
| `Call`                | `0x1C`          | Calls the function at #`Function` in the function table. Pushes the Locals stack pointer onto the local stack as 8 bytes. Pushes the Locals head onto the local stack as 8 bytes. Pushes the address of the next instruction onto the local stack as 8 bytes. | Function `<u32>`         |                 |                |
| `Call.Ext`            | `0x801C`        | Calls external function #`Function` in function table of module #`Module`. Pushes the Locals stack pointer onto the local stack as 8 bytes. Pushes the Locals head onto the local stack as 8 bytes. Pushes the address of the next instruction onto the local stack as 8 bytes. | Function `<u32>`         | Module `<u16>`  |                |
| `Call.Ind`            | `0x801D`        | Uses the local `Function Pointer` as an index for the function table and jumps to the location. `Function Pointer` is a 4 byte index, but must have a function pointer type. Pushes the Locals stack pointer onto the local stack as 8 bytes. Pushes the Locals head onto the local stack as 8 bytes. Pushes the address of the next instruction onto the local stack as 8 bytes. | Function Pointer `<u16>` |                 |                |
| `Call.Ind.Ext`        | `0x801E`        | Uses the local `Function Pointer` as an index for the function table. `Function Pointer` is a 4 byte index, but must have a function pointer type. Pops off 2 bytes from the stack as a `Module` index for the module table. Jumps to the function. Pushes the Locals stack pointer onto the local stack as 8 bytes. Pushes the Locals head onto the local stack as 8 bytes. Pushes the address of the next instruction onto the local stack as 8 bytes. | Function Pointer `<u16>` |                 |                |
| `Ret`                 | `0x1D`          | Pops 8 bytes off the local stack and jumps as an address and jumps to location. |                          |                 |                |
| `Jump`                | `0x1E`          | Performs a unconditional jump by `Offset` bytes. Offset starts from after the instruction. | Offset `<i32>`           |                 |                |
| `Jump.True`           | `0x70`          | Pops 1 byte off the stack into register `Condition`. Performs a conditional jump by `Offset` bytes, if the value in `Condition` is not 0. Offset starts after this instructions bytes. | Offset `<i32>`           |                 |                |
| `Jump.False`          | `0x71`          | Pops 1 byte off the stack into register  `Condition`. Performs a conditional jump by `Offset` bytes, if the value in `Condition` is 0. Offset starts after this instructions bytes. | Offset `<i32>`           |                 |                |

### Jump/Compare Conditions

| Name                | Encoding | Description    |
| ------------------- | -------- | -------------- |
| `Above`             | `0x0`    | CF=0 and ZF=0  |
| `AboveOrEqual`      | `0x1`    | CF=0           |
| `Below`             | `0x2`    | CF=1           |
| `BelowOrEqual`      | `0x3`    | CF=1 or ZF=1   |
| `Carry`             | `0x2`    | CF=1           |
| `Equal`             | `0x4`    | ZF=1           |
| `Greater`           | `0x5`    | ZF=0 and SF=OF |
| `GreaterOrEqual`    | `0x6`    | SF=OF          |
| `Less`              | `0x7`    | SF≠OF          |
| `LessOrEqual`       | `0x8`    | ZF=1 or SF≠OF  |
| `NotAbove`          | `0x3`    | CF=1 or ZF=1   |
| `NotAboveOrEqual`   | `0x2`    | CF=1           |
| `NotBelow`          | `0x1`    | CF=0           |
| `NotBelowOrEqual`   | `0x0`    | CF=0 and ZF=0  |
| `NotCarry`          | `0x1`    | CF=0           |
| `NotEqual`          | `0x9`    | ZF=0           |
| `NotGreater`        | `0x8`    | ZF=1 or SF≠OF  |
| `NotGreaterOrEqual` | `0x7`    | SF≠OF          |
| `NotLess`           | `0x6`    | SF=OF          |
| `NotLessOrEqual`    | `0x5`    | ZF=0 and SF=OF |
| `NotOverflow`       | `0xA`    | OF=0           |
| `NotParity`         | `0xB`    | PF=0           |
| `NotSign`           | `0xC`    | SF=0           |
| `NotZero`           | `0x9`    | ZF=0           |
| `Overflow`          | `0xD`    | OF=1           |
| `Parity`            | `0xE`    | PF=1           |
| `ParityEven`        | `0xE`    | PF=1           |
| `ParityOdd`         | `0xB`    | PF=0           |
| `Sign`              | `0xF`    | SF=1           |
| `Zero`              | `0x4`    | ZF=1           |

### Flags Register

| Flag Name | Bit Index | Mask     | Description   | =1               | =0                |
| --------- | --------- | -------- | ------------- | ---------------- | ----------------- |
| `CF`      | 0         | `0x0001` | Carry Flag    | CY (Carry)       | NC (No Carry)     |
|           | 1         | `0x0002` | Reserved      |                  |                   |
| `PF`      | 2         | `0x0004` | Parity Flag   | PE (Parity Even) | PO (Parity Odd)   |
|           | 3         | `0x0008` | Reserved      |                  |                   |
|           | 4         | `0x0010` | Reserved      |                  |                   |
|           | 5         | `0x0020` | Reserved      |                  |                   |
| `ZF`      | 6         | `0x0040` | Zero Flag     | ZR (Zero)        | NZ (Not Zero)     |
| `SF`      | 7         | `0x0080` | Sign Flag     | NG (Negative)    | PL (Positive)     |
|           | 8         | `0x0100` | Reserved      |                  |                   |
|           | 9         | `0x0200` | Reserved      |                  |                   |
|           | 10        | `0x0400` | Reserved      |                  |                   |
| `OF`      | 11        | `0x0800` | Overflow Flag | OV (Overflow)    | NV (Not Overflow) |
|           | 12        | `0x1000` | Reserved      |                  |                   |
|           | 13        | `0x2000` | Reserved      |                  |                   |
|           | 14        | `0x4000` | Reserved      |                  |                   |
|           | 15        | `0x8000` | Reserved      |                  |                   |

## SSA

The **S**ingle **S**tatic **A**ssignment assembly is used during the JIT process.

### SSA Variables

Because this is Single Static Assignment, every variable is immutable and can only be set once. If the variable ID has the high bit set, then it is a function parameter. Functions always start their parameters at 0 (with the high bit set, i.e. 0x80000000). A function cannot mutate the values in its parameters. When a function call is made, the base index (without the high bit) is passed into the call. Every variable sequentially at and after the base index variable is then passed into the function, at which point they would be localized to offset 0 (0x80000000) automatically.

### SSA Types

Basic types are encoded with a single byte. If the high byte is set it is a pointer type. If the value is `0x7F` (or `0xFF` for pointers) then it is a custom type, and will be followed by a 4 byte `u32` containing the index within the type table. If the value is `0x7E` (or `0xFE` for pointers) then it is a raw byte buffer and will be followed by a 4 byte `u32` containing the number bytes in the buffer.

| Type Name | Encoding |
| --------- | -------- |
| `void`    | `0x00`   |
| `bool`    | `0x01`   |
| `i8`      | `0x02`   |
| `i16`     | `0x03`   |
| `i32`     | `0x04`   |
| `i64`     | `0x05`   |
| `u8`      | `0x06`   |
| `u16`     | `0x07`   |
| `u32`     | `0x08`   |
| `u64`     | `0x09`   |
| `f16`     | `0x0A`   |
| `f32`     | `0x0B`   |
| `f64`     | `0x0C`   |
| `char`    | `0x0D`   |
| `bytes`   | `0x7E`   |
| Custom    | `0x7F`   |

### SSA Binary Ops

| Operation          | Encoding | Description                               |
| ------------------ | -------- | ----------------------------------------- |
| `Add`              | `0x00`   | Add `B` to `A`.                           |
| `Sub`              | `0x01`   | Subtract `B` from `A`.                    |
| `Mul`              | `0x02`   | Multiply `A` by `B`.                      |
| `Div`              | `0x03`   | Divide `A` by `B`.                        |
| `Rem`              | `0x04`   | Divide `A` by `B` and keep the remainder. |
| `BitShiftLeft`     | `0x05`   | Bit shift left `A` by `B`.                |
| `BitShiftRight`    | `0x06`   | Bit shift right `A` by `B`.               |
| `BarrelShiftLeft`  | `0x07`   | Barrel shift left `A` by `B`.             |
| `BarrelShiftRight` | `0x08`   | Barrel shift right `A` by `B`.            |



### SSA Opcodes

| Opcode             | Encoding | Description                                                  | Operand 0               | Operand 1             | Operand 2             | Operand 3            |
| ------------------ | -------- | ------------------------------------------------------------ | ----------------------- | --------------------- | --------------------- | -------------------- |
| `Nop`              | `0x00`   | Does nothing, should never be used.                          |                         |                       |                       |                      |
| `Label`            | `0x01`   | Registers a new label. Labels use the same indexing as variables. |                         |                       |                       |                      |
| `Assign.Immediate` | `0x30`   | Create new variable of `New Type`, and assign a value `Value`. | New Type `<Type>`       | Value `<New Type>`    |                       |                      |
| `Assign.Variable`  | `0x31`   | Create new variable of `New Type`, and assign its value from variable #`Var`. | New Type `<Type>`       | Var `<u32>`           |                       |                      |
| `Expand.SX`        | `0x32`   | Create new variable of `New Type`, and assign its value from the sign extended variable #`Var` of `Old Type`. | New Type `<Type>`       | Old Type `<Type>`     | Var `<u32>`           |                      |
| `Expand.ZX`        | `0x33`   | Create new variable of `New Type`, and assign its value from the zero extended variable #`Var` of `Old Type`. | New Type `<Type>`       | Old Type `<Type>`     | Var `<u32>`           |                      |
| `Trunc`            | `0x34`   | Create new variable of `New Type`, and assign its value from the truncated variable #`Var` of `Old Type`. | New Type `<Type>`       | Old Type `<Type>`     | Var `<u32>`           |                      |
| `RCast`            | `0x36`   | Reinterpret cast variable #`Var` from `Old Type` to `New Type` and store it in a new variable. | New Type `<Type>`       | Old Type `<Type>`     | Var `<u32>`           |                      |
| `BCast`            | `0x37`   | Bit cast variable #`Var` from `Old Type` to `New Type` and store it in a new variable. | New Type `<Type>`       | Old Type `<Type>`     | Var `<u32>`           |                      |
| `Load`             | `0x38`   | Create a new variable of type `Type` and fill it with the value pointed to by variable #`Var`. | Type `<Type>`           | Var `<u32>`           |                       |                      |
| `Store`            | `0x39`   | Store variable #`Source` of type `Type` into the address pointed to by variable #`Destination` | Type `<Type>`           | Destination `<u32>`   | Source `<u32>`        |                      |
| `Store`            | `0x3B`   | Store immediate #`Value` of type `Data Type` into the address pointed to by variable #`Destination` | Date Type `<Type>`      | Destination `<u32>`   | Value `<Data Type>`   |                      |
| `ComputePtr`       | `0x3A`   | Compute a pointer and store it in a new variable. The formula is `%Base + %Index * $Multiplier + $Offset`.  To compute `%Index * $Multiplier + $Offset` set #`Base` to #`Index` and subtract one from `Multiplier`. To compute `%Base + $Offset` set #`Index` to #`Base` and set `Multiplier` to 0. | Base `<u32>`            | Index `<u32>`         | Multiplier `<i8>`     | Offset `<i16>`       |
| `BinOp`            | `0x50`   | Perform operation `Operation` with variable #`B` to variable #`A` and store it in a new variable. | Operation `<BinOp>`     | Data Type `<Type>`    | A `<u32>`             | B `<u32>`            |
| `BinOp`            | `0x51`   | Perform operation `Operation` with variable #`B` to immediate `A` and store it in a new variable. | Operation `<BinOp>`     | Data Type `<Type>`    | A `<Data Type>`       | B `<u32>`            |
| `BinOp`            | `0x52`   | Perform operation `Operation` with immediate `B` to variable #`A` and store it in a new variable. | Operation `<BinOp>`     | Data Type `<Type>`    | A `<u32>`             | B `<Data Type>`      |
| `Split`            | `0x20`   | Split `A` into `N` new variables with the types `T0..TN-1` with `T0` being the highest bits, and `TN-1` being the lowest bits. This will copy bytes raw.  All types must be integral. | `A` Data Type `<Type>`  | A `<u32>`             | N `<u32>`             | T0..TN-1 `<Type[N]>` |
| `Join`             | `0x21`   | Join `N` variables `V0..VN-1` of types `T0..TN-1` into a new variable of `Out Type` with `V0` being the highest bits and `VN-1` being the lowest bits. All types must be integral. | Out Type `<Type>`       | N `<u32>`             | T0..TN-1 `<Type[N]>`  | V0..VN-1 `<u32[N]>`  |
| `Comp`             | `0x70`   | Compare variable  #`B` to variable #`A` using the specified Compare Condition. | Condition `<Condition>` | Data Type `<Type>`    | A `<u32>`             | B `<u32>`            |
| `Comp`             | `0x71`   | Compare variable  #`B` to immediate `A` using the specified Compare Condition. | Condition `<Condition>` | Data Type `<Type>`    | A `<Data Type>`       | B `<u32>`            |
| `Comp`             | `0x72`   | Compare immediate `B` to variable #`A` using the specified Compare Condition. | Condition `<Condition>` | Data Type `<Type>`    | A `<u32>`             | B `<Data Type>`      |
| `Branch`           | `0x40`   | Jump to label `Label`.                                       | Label `<Label>`         |                       |                       |                      |
| `Branch.Cond`      | `0x41`   | Jumps to label `Label True` if `Condition` is not 0, otherwise jumps to label `Label False`. | Label True`<Label>`     | Label False `<Label>` | Condition Var `<u32>` |                      |
| `Call`             | `0x42`   | Calls the function with index `Function`. Passes `Parameter Count` parameters after `Base Index` to the function. `Parameter Count` is useful for variadic functions. | Function `u32`          | Base Index `u32`      | Parameter Count `u32` |                      |
| `Call.Ext`         | `0x43`   | Calls the function with index `Function` in module `Module`. Passes `Parameter Count` parameters after `Base Index` to the function. `Parameter Count` is useful for variadic functions. | Function `u32`          | Base Index `u32`      | Parameter Count `u32` | Module `u16`         |
| `Call.Ind`         | `0x44`   | Calls the function with index stored in `Function Pointer`. Passes `Parameter Count` parameters after `Base Index` to the function. `Parameter Count` is useful for variadic functions. | Function Pointer`u32`   | Base Index `u32`      | Parameter Count `u32` |                      |
| `Call.Ind`         | `0x45`   | Calls the function with index stored in `Function Pointer` in mo. Passes `Parameter Count` parameters after `Base Index` to the function. `Parameter Count` is useful for variadic functions. | Function Pointer`u32`   | Base Index `u32`      | Parameter Count `u32` | Module Pointer `u32` |
| `Ret`              | `0x46`   |                                                              | `A` Data Type `<Type>`  | Value `<u32>`         |                       |                      |
|                    |          |                                                              |                         |                       |                       |                      |

