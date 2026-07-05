# Synapse-32

Synapse-32 is a 32-bit RISC-V CPU core written in Verilog, supporting RV32I instructions, along with Zicsr and Zifencei extensions.

## What is RISC-V
  RISC-V [Reduced Instruction Set Computer(pronounced "risk five")] is an open-source Instruction Set Architecture (ISA). It defines what instructions a CPU understands, but not how the CPU is built.
  The idea is:
  - Use fewer, simpler instructions
  - Execute them quickly
  - Let software combine simple instructions to perform complex tasks

    ![image.png](https://anysilicon.com/wp-content/uploads/2022/07/risc-v.jpg)

## Processor Architecture

### 5-Stage Pipeline
This processor implements a classic 5-stage RISC pipeline:

1. **IF (Instruction Fetch)**: 
   - Fetches the next instruction from instruction memory
   - Updates the Program Counter (PC)

2. **ID (Instruction Decode)**:
   - Decodes the instruction
   - Reads values from register file
   - Generates immediate values and control signals

3. **EX (Execute)**:
   - Performs ALU operations
   - Calculates branch/jump addresses
   - Makes branch decisions

4. **MEM (Memory Access)**:
   - Performs memory reads and writes
   - Handles load and store instructions

5. **WB (Write Back)**:
   - Writes results back to the register file
   - Selects appropriate data source (ALU or memory)

### Pipeline Hazards
#### A **pipeline hazard** is a situation where the CPU cannot execute the next instruction as planned, causing a **stall** or **delay** in the pipeline.
There are **three types of pipeline hazards**.
---
#### 1. Structural Hazard
A **structural hazard** occurs when **two instructions need the same hardware resource at the same time**.
*Example*
Instruction 1 → MEM (uses memory)
Instruction 2 → IF  (fetches instruction)

Both require the same memory.
Since only one memory is available, one instruction must wait.
**Solution:** Use separate instruction and data memory or duplicate the hardware.

---

#### 2. Data Hazard

#### A **data hazard** occurs when an instruction depends on the result of a previous instruction that has not finished executing.
*Example*
assembly
ADD x3, x1, x2
SUB x4, x3, x5

SUB needs the updated value of `x3`, but `ADD` has not written it back yet.
**Solution:** Use **Forwarding (Bypassing)** or **stall the pipeline** until the data is available. 
---
#### 3. Control Hazard

A **control hazard** occurs when the CPU does not know which instruction to execute next because of a **branch** or **jump** instruction.

*Example*

assembly
BEQ x1, x2, LOOP
ADD x3, x4, x5
If `x1 == x2`, the CPU jumps to `LOOP`, so the `ADD` instruction should not execute. However, the CPU may have already fetched it before knowing the branch result.

**Solution:** Use **pipeline flush**, **branch prediction**, or **stall** until the branch decision is known.

---

5 STAGE PIPELINE IN WORK

![image.png](https://media.geeksforgeeks.org/wp-content/uploads/20250502142202506624/instruction_execution_in_5_stage_pipeline.webp)

<img src="https://github.com/amrit-reboot/Reforge/blob/Amrit/Docs/Image/control-hazard.png" >

#### Summary

| Hazard | Cause | Example | Solution |
|---------|-------|---------|----------|
| **Structural** | Same hardware needed by multiple instructions | IF and MEM using the same memory | Separate hardware or stall |
| **Data** | Instruction depends on previous result | `ADD` → `SUB` | Forwarding or stall |
| **Control** | Branch or jump changes program flow | `BEQ`, `JAL` | Branch prediction, flush, or stall |

> **Key Idea:** Pipeline hazards prevent smooth instruction execution. Modern processors reduce their impact using forwarding, branch prediction, pipeline flushing, and better hardware design.

------
------
# PROJECT OVERVIEW
This project was created in order to create a RISC-V CPU ona an Upduino 3.0 FPGA using verilog HDL.
My AIM is to learn the concepts of RISC-V and how it works, apart form that i aim to get a decent grasp of comp-arch and try to understand this project.

THIS PROJECT BELONGS TO SRA-VJTI and here it is just being used to study and research purpose, theres no misuse of data and no plagarism intended.

## Workflow
This is a *2 STAGE PROCESSOR*.

FIRST STAGE:
- Instructions are fed and decoded
- Values are read from register file
- Sending necessary parameters to ALU if needed
- Writing in DMem and sending read signals
- Send jump to PC

SECOND STAGE:
- Get ALU output
- Read data from Dmem
- Control unit writes to register file
- PC executes jump instruction
- Show output on *SEVEN SEGMENT DISPLAY*


<img src="Docs/Image/Workflow_synapse32.png" >





