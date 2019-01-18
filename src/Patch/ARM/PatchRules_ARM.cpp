/*
 * This file is part of QBDI.
 *
 * Copyright 2017 Quarkslab
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Patch/PatchRule.h"
#include "Patch/ARM/PatchRules_ARM.h"
#include "Patch/ARM/RelocatableInst_ARM.h"
#include "Patch/ARM/Layer2_ARM.h"

namespace QBDI {

RelocatableInst::SharedPtrVec getExecBlockPrologue() {
    RelocatableInst::SharedPtrVec prologue;
    //prologue.push_back({BreakPoint(CPUMode::ARM)});

    // Save return address on host stack:
    // STR LR, [sp, #-4]!
    prologue.push_back(Pushr(Reg(REG_LR)));

    // Save host fp and sp:
    // STR SP, [pc, #OFF] <-- OFF is the offset of SP in DataBlock Context
    append(prologue, SaveReg(Reg(REG_SP), Offset(offsetof(Context, hostState.sp))).generate((CPUMode) 0));

    // Move sp at the beginning of the data block to solve memory addressing range problems
    // This instruction needs to be EXACTLY HERE for relative addressing alignement reasons
    prologue.push_back(Adr((CPUMode) 0, Reg(REG_SP), 0xff0));
    append(prologue, SaveReg(Reg(REG_BP), Offset(offsetof(Context, hostState.fp))).generate((CPUMode) 0));

    // Restore FPR
    for (unsigned int i = 0; i < QBDI_NUM_FPR; i++) {
      // vldr	si, [sp, OFF]
      prologue.push_back(
          Vldrs(llvm::ARM::S0 + i, Reg(REG_SP), offsetof(Context, fprState.s) + i * sizeof(float))
      );
    }

    // Restore CPSR
    // ldr r0, [sp, #220]
    prologue.push_back(Ldr(CPUMode::ARM, Reg(0), Reg(REG_SP), offsetof(Context, gprState.cpsr)));
    prologue.push_back(Msr(Reg(0)));

    // Restore GPR
    // R0 ... LR
    // ldr R0, [pc, DataBlock(R0)]
    // ...
    // ldr LR, [pc, DataBlock(LR)]
    for (unsigned int i = 0; i < NUM_GPR - 1; i++) {
      append(prologue, LoadReg(Reg(i), Offset(Reg(i))).generate((CPUMode) 0));
    }

    // Jump selector:
    // ldr PC, [pc, DataBlock(HostState.selector)]
    prologue.push_back(
        Ldr(CPUMode::ARM, Reg(REG_PC), Offset(offsetof(Context, hostState.selector)))
    );

    return prologue;
}

RelocatableInst::SharedPtrVec getExecBlockEpilogue() {
    RelocatableInst::SharedPtrVec epilogue;

    // Save guest state
    // R0 .. SP
    for (unsigned int i = 0; i < NUM_GPR - 2; i++) {
      append(epilogue, SaveReg(Reg(i), Offset(Reg(i))).generate((CPUMode) 0));
    }

    // Move sp at the beginning of the data block to solve memory addressing range problems
    epilogue.push_back(Adr(CPUMode::ARM, Reg(REG_SP), Offset(0)));

    // Save FPR
    for (unsigned int i = 0; i < QBDI_NUM_FPR; i++) {
      epilogue.push_back(
          Vstrs(llvm::ARM::S0 + i, Reg(REG_SP), offsetof(Context, fprState.s) + i * sizeof(float))
      );
    }

    // Save CPSR
    epilogue.push_back(Mrs(Reg(0)));
    epilogue.push_back(Str(CPUMode::ARM, Reg(0), Offset(offsetof(Context, gprState.cpsr))));

    // Restore host BP, SP
    append(epilogue, LoadReg(Reg(REG_BP), Offset(offsetof(Context, hostState.fp))).generate((CPUMode) 0));
    append(epilogue, LoadReg(Reg(REG_SP), Offset(offsetof(Context, hostState.sp))).generate((CPUMode) 0));
    //append(epilogue, {BreakPoint(CPUMode::ARM)});
    // Return to host
    epilogue.push_back(Popr(Reg(REG_PC)));

    return epilogue;
}

PatchRule::SharedPtrVec getDefaultPatchRules() {
    PatchRule::SharedPtrVec rules;

    /* Rule #0: Simulating BX PC instructions.
     * Target:  BX PC
     * Patch:   Temp(0) := Operand(0)
     *          DataOffset[Offset(PC)] := Temp(0)
    */
    rules.push_back(
        PatchRule(
            And({
                Or({
                    OpIs(llvm::ARM::BX),
                    OpIs(llvm::ARM::BX_pred),
                    OpIs(llvm::ARM::tBX),
                }),
                UseReg(REG_PC)
            }),
            {
                GetPCOffset(Temp(0), Constant(0)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC))),
                SimulateExchange(Temp(0))
            }
        )
    );

    /* Rule #1: Simulating BX instructions.
     * Target:  BX REG
     * Patch:   Temp(0) := Operand(0)
     *          DataOffset[Offset(PC)] := Temp(0)
    */
    rules.push_back(
        PatchRule(
            Or({
                OpIs(llvm::ARM::BX),
                OpIs(llvm::ARM::BX_pred),
                OpIs(llvm::ARM::tBX),
            }),
            {
                GetOperand(Temp(0), Operand(0)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC))),
                SimulateExchange(Temp(0))
            }
        )
    );

    /* Rule #2: Simulating BLX instructions.
     * Target:  BLX REG
     * Patch:   Temp(0) := Operand(0)
     *          DataOffset[Offset(PC)] := Temp(0)
     *          SimulateLink(Temp(0))
    */
    rules.push_back(
        PatchRule(
            Or({
                OpIs(llvm::ARM::BLX),
                OpIs(llvm::ARM::BLX_pred),
            }),
            {
                GetOperand(Temp(0), Operand(0)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC))),
                SimulateLink(Temp(0)),
                SimulateExchange(Temp(0))
            }
        )
    );

    /* Rule #3: Simulating BL immediate instructions for ARM.
     * Target:  BL(X) IMM
     * Patch:   Temp(0) := PC + Operand(0)
     *          DataOffset[Offset(PC)] := Temp(0)
     *          SimulateLink(Temp(0))
    */
    rules.push_back(
        PatchRule(
            Or({
                OpIs(llvm::ARM::BL),
                OpIs(llvm::ARM::BL_pred),
            }),
            {
                GetPCOffset(Temp(0), Operand(0)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC))),
                SimulateLink(Temp(0))
            }
        )
    );

    /* Rule #4: Simulating BLX immediate instructions for ARM.
     * Target:  BL(X) IMM
     * Patch:   Temp(0) := PC + Operand(0)
     *          DataOffset[Offset(PC)] := Temp(0)
     *          SimulateLink(Temp(0))
    */
    rules.push_back(
        PatchRule(
            OpIs(llvm::ARM::BLXi),
            {
                GetPCOffset(Temp(0), Operand(0)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC))),
                SimulateLink(Temp(0)),
                SimulateExchange(Temp(0)),
            }
        )
    );

    /* Rule #5: Simulating BL immediate instructions for Thumb.
     * Target:  BL IMM
     * Patch:   Temp(0) := PC + Operand(2)
     *          DataOffset[Offset(PC)] := Temp(0)
     *          SimulateLink(Temp(0))
    */
    rules.push_back(
        PatchRule(
            Or({
                OpIs(llvm::ARM::tBL),
            }),
            {
                GetPCOffset(Temp(0), Operand(2)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC))),
                SimulateLink(Temp(0))
            }
        )
    );

    /* Rule #6: Simulating BL immediate instructions for Thumb.
     * Target:  BLX IMM
     * Patch:   Temp(0) := PC + Operand(2)
     *          DataOffset[Offset(PC)] := Temp(0)
     *          SimulateLink(Temp(0))
     *          SimulateExchange(Temp(0)),
    */
    rules.push_back(
        PatchRule(
            Or({
                OpIs(llvm::ARM::tBLXi),
            }),
            {
                GetPCOffset(Temp(0), Operand(2)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC))),
                SimulateLink(Temp(0)),
                SimulateExchange(Temp(0)),
            }
        )
    );

    /* Rule #7: Simulating B immediate instructions for Thumb.
     * Target:  B IMM
     * Patch:   Temp(0) := PC + Operand(2)
     *          DataOffset[Offset(PC)] := Temp(0)
    */
    rules.push_back(
        PatchRule(
            Or({
                OpIs(llvm::ARM::tB),
                OpIs(llvm::ARM::t2B),
            }),
            {
                GetPCOffset(Temp(0), Operand(0)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC))),
            }
        )
    );

    /* Rule #8: Simulating BX_RET and MOVPCLR with conditional flag.
     * Target:  BXcc LR | MOVcc PC, LR
     * Patch:   Temp(0) := PC + Constant(-4) # next instruction address
     *          (BXcc LR | MOVcc PC, LR) --> MOVcc Temp(0), LR
     *          DataOffset[Offset(PC)] := Temp(0)
    */
    rules.push_back(
        PatchRule(
            Or({
                OpIs(llvm::ARM::MOVPCLR),
                OpIs(llvm::ARM::BX_RET),
            }),
            {
                GetPCOffset(Temp(0), Constant(-4)),
                ModifyInstruction({
                    SetOpcode(llvm::ARM::MOVr),
                    AddOperand(Operand(0), Temp(0)),
                    AddOperand(Operand(1), Reg(REG_LR)),
                    AddOperand(Operand(4), Constant(0)),
                }),
                WriteTemp(Temp(0), Offset(Reg(REG_PC))),
                SimulateExchange(Temp(0)),
            }
        )
    );

    /* Rule #9: Simulating B with conditional flag under ARM.
     * Target:  Bcc IMM
     * Patch:     Temp(0) := PC + Operand(0)
     *         ---Bcc IMM --> Bcc END
     *         |  Temp(0) := PC + Constant(-4) # next instruction
     *         -->END: DataOffset[Offset(PC)] := Temp(0)
    */
    rules.push_back(
        PatchRule(
            OpIs(llvm::ARM::Bcc),
            {
                GetPCOffset(Temp(0), Operand(0)),
                ModifyInstruction({
                    SetOperand(Operand(0), Constant(0))
                }),
                GetPCOffset(Temp(0), Constant(-4)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC)))
            }
        )
    );

    /* Rule #10: Simulating B with conditional flag under thumb.
     * Target:  Bcc IMM
     * Patch:     Temp(0) := PC + Operand(0)
     *         ---Bcc IMM --> Bcc END
     *         |  Temp(0) := PC + Constant(-4) # next instruction
     *         -->END: DataOffset[Offset(PC)] := Temp(0)
    */
    rules.push_back(
        PatchRule(
            OpIs(llvm::ARM::tBcc),
            {
                GetPCOffset(Temp(0), Operand(0)),
                ModifyInstruction({
                    SetOperand(Operand(0), Constant(2))
                }),
                GetPCOffset(Temp(0), Constant(-2)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC)))
            }
        )
    );

    /* Rule #11: Simulating CBZ with conditional flag.
     * Target:  Bcc IMM
     * Patch:     Temp(0) := PC + Operand(1)
     *         ---CBZ IMM --> Bcc END
     *         |  Temp(0) := PC + Constant(-4) # next instruction
     *         -->END: DataOffset[Offset(PC)] := Temp(0)
    */
    rules.push_back(
        PatchRule(
            Or({
                OpIs(llvm::ARM::tCBZ),
                OpIs(llvm::ARM::tCBNZ),
            }),
            {
                GetPCOffset(Temp(0), Operand(1)),
                ModifyInstruction({
                    SetOperand(Operand(1), Constant(2))
                }),
                GetPCOffset(Temp(0), Constant(-2)),
                WriteTemp(Temp(0), Offset(Reg(REG_PC)))
            }
        )
    );

    /* Rule #12: Simulating LDMIA using pc in the reg list.
     * Target:  LDMIA {REG1, ..., REGN, pc}
     * Patch:   LDMIA {REG1, ..., REGN, pc} --> LDMIA {REG1, ..., REGN}
     *          SimulateRet(Temp(0))
    */
    rules.push_back(
        PatchRule(
            And({
                OpIs(llvm::ARM::LDMIA_UPD),
                UseReg(Reg(REG_PC)),
            }),
            {
                ModifyInstruction({
                    RemoveOperand(Reg(REG_PC))
                }),
                SimulatePopPC(Temp(0)),
                SimulateExchange(Temp(0)),
            }
        )
    );

    /* Rule #13:
      pop	{r4, r5, r6, r7, pc}
    */
    rules.push_back(
        PatchRule(
            And({
                OpIs(llvm::ARM::tPOP),
                UseReg(Reg(REG_PC)),
            }),
            {
                ModifyInstruction({
                    RemoveOperand(Reg(REG_PC))
                }),
                SimulatePopPC(Temp(0)),
                //SimulateExchange(Temp(0)),
            }
        )
    );

    /* Rule #14: Generic PC modification patch with potential conditional code.
     * Target:  Anything that has PC as destination operand. E.g. ADDcc PC, PC, R1
     * Patch:   Temp(0) := PC + Constant(0)  # to substitute read values of PC
     *          Temp(1) := PC + Constant(-4) # to substitute written values of PC
     *          ADDcc PC, PC, R1 --> ADDcc Temp(1), Temp(0), R1
     *          DataOffset[Offset(PC)] := Temp(1)
    */
    rules.push_back(
        PatchRule(
            RegIs(Operand(0), Reg(REG_PC)),
            {
                GetPCOffset(Temp(0), Constant(0)),
                GetPCOffset(Temp(1), Constant(-4)),
                ModifyInstruction({
                    SubstituteWithTemp(Reg(REG_PC), Temp(0)),
                    SetOperand(Operand(0), Temp(1))
                }),
                WriteTemp(Temp(1), Offset(Reg(REG_PC))),
                SimulateExchange(Temp(0)),
            }
        )
    );



    /* Rule #15: Generic PC utilization patch
     * Target:  Anything that uses PC. E.g. ADDcc R2, PC, R1
     * Patch:   Temp(0) := PC + Constant(0)  # to substitute read values of PC
     *          ADDcc R2, PC, R1 --> ADDcc R2, Temp(0), R1
    */
    rules.push_back(
        PatchRule(
            UseReg(Reg(REG_PC)),
            {
                GetPCOffset(Temp(0), Constant(0)),
                ModifyInstruction({
                    SubstituteWithTemp(Reg(REG_PC), Temp(0))
                }),
            }
        )
    );

    /* Rule #16:
     * Target:  LDR REG, [pc, OFF] (Thumb mode)
     * Patch:   Temp(0) := PC + Constant(0)
     *          LDR REG, [Temp(0), OFF]
    */
    rules.push_back(
        PatchRule(
            OpIs(llvm::ARM::tLDRpci),
            {
                GetPCOffset(Temp(0), Constant(0)),
                ModifyInstruction({
                    TestTransform(Temp(0)),
                }),
            }
        )
    );


    /* Rule #17: Default rule for every other instructions.
     * Target:   *
     * Patch:    Output original unmodified instructions.
    */
    rules.push_back(PatchRule(True(), {ModifyInstruction({})}));

    return rules;
}

// Patch allowing to terminate a basic block early by writing address into DataBlock[Offset(PC)]
RelocatableInst::SharedPtrVec getTerminator(rword address, CPUMode cpuMode) {
    RelocatableInst::SharedPtrVec terminator;

    append(terminator, SaveReg(Reg(2), Offset(Reg(2))).generate(cpuMode));
    terminator.push_back(Ldr(cpuMode, Reg(2), Constant(address)));
    append(terminator, SaveReg(Reg(2), Offset(Reg(REG_PC))).generate(cpuMode));
    append(terminator, LoadReg(Reg(2), Offset(Reg(2))).generate(cpuMode));

    return terminator;
}

}
