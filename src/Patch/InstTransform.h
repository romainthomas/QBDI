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
#ifndef INSTTRANSFORM_H
#define INSTTRANSFORM_H

#include <memory>
#include <vector>

#include "llvm/MC/MCInst.h"

#include "Patch/Types.h"
#include "Patch/TempManager.h"
#include "Patch/PatchUtils.h"

namespace QBDI {

class InstTransform {
  public:

    using SharedPtr    = std::shared_ptr<InstTransform>;
    using SharedPtrVec = std::vector<std::shared_ptr<InstTransform>>;

    virtual void transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) = 0;

    virtual ~InstTransform();
};

class SetOperand : public InstTransform, public AutoAlloc<InstTransform, SetOperand> {
    Operand opn;
    enum {
        TempOperandType,
        RegOperandType,
        ImmOperandType
    } type;
    // Not working VS 2015
    // union {
        Temp temp;
        Reg reg;
        Constant imm;
    // };

  public:
    /*! Set the operand opn of the instruction as the Temp temp.
     *
     * @param[in] opn   Operand index in the LLVM MCInst representation.
     * @param[in] temp  Temporary register which will be set as the new operand
    */
    SetOperand(Operand opn, Temp temp);

    /*! Set the operand opn of the instruction as the Reg reg.
     *
     * @param[in] opn  Operand index in the LLVM MCInst representation.
     * @param[in] reg  Register which will be set as the new operand.
    */
    SetOperand(Operand opn, Reg reg);

    /*! Set the operand opn of the instruction as the immediate imm.
     *
     * @param[in] opn  Operand index in the LLVM MCInst representation.
     * @param[in] imm  Constant which will be set as the new immediate operand.
    */
    SetOperand(Operand opn, Constant imm);

    virtual void transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) override;
};

class SubstituteWithTemp : public InstTransform,
                           public AutoAlloc<InstTransform, SubstituteWithTemp> {
    Reg  reg;
    Temp temp;

  public:

    /*! Substitute every reference to reg in the operands of the instruction with temp.
     *
     * @param[in] reg   Register which will be substituted.
     * @param[in] temp  Temporary register which will be substituted with.
    */
    SubstituteWithTemp(Reg reg, Temp temp);

    virtual void transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) override;
};

class AddOperand : public InstTransform, public AutoAlloc<InstTransform, AddOperand> {

    Operand opn;
    enum {
        TempOperandType,
        RegOperandType,
        ImmOperandType
    } type;
    // Not working under VS2015
    // union {
        Temp temp;
        Reg reg;
        Constant imm;
    // };

public:

    /*! Add a new temporary register operand to the instruction by inserting it at operand index opn.
     *
     * @param[in] opn   Operand index in LLVM MCInst representation.
     * @param[in] temp  Temp to be inserted as a new operand.
    */
    AddOperand(Operand opn, Temp temp);

    /*! Add a new register operand to the instruction by inserting it at operand index opn.
     *
     * @param[in] opn  Operand index in LLVM MCInst representation.
     * @param[in] reg  Register to be inserted as a new operand.
    */
    AddOperand(Operand opn, Reg reg);

    /*! Add a new immediate operand to the instruction by inserting it at operand index opn.
     *
     * @param[in] opn  Operand index in LLVM MCInst representation.
     * @param[in] imm  Constant to be inserted as a new immediate operand.
    */
    AddOperand(Operand opn, Constant imm);

    virtual void transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) override;
};

class RemoveOperand : public InstTransform, public AutoAlloc<InstTransform, RemoveOperand> {

    Reg reg;

  public:

    /*! Remove the first occurence of reg in the operands of the instruction.
     *
     * @param[in] reg Register to remove from the operand list.
    */
    RemoveOperand(Reg reg);

    virtual void transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) override;
};


class SetOpcode : public InstTransform, public AutoAlloc<InstTransform, SetOpcode> {

    unsigned int opcode;

  public:

    /*! Set the opcode of the instruction.
     *
     * @param[in] opcode New opcode to set as the instruction opcode.
    */
    SetOpcode(unsigned int opcode);

    virtual void transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) override;
};


}

#endif
