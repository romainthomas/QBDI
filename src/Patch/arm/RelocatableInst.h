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
#ifndef RELOCATABLEINST_ARM_H
#define RELOCATABLEINST_ARM_H

#include "Patch/RelocatableInst.h"

namespace QBDI {

class MemoryConstant: public RelocatableInst, public AutoAlloc<RelocatableInst, MemoryConstant> {
    unsigned int opn;
    rword        value;

  public:
    MemoryConstant(llvm::MCInst inst, unsigned int opn, rword value);
    llvm::MCInst reloc(ExecBlock *execBlock, CPUMode cpuMode);
};

class DataBlockRel : public RelocatableInst, public AutoAlloc<RelocatableInst, DataBlockRel> {
    unsigned int opn;
    rword        offset;

  public:
    DataBlockRel(llvm::MCInst inst, unsigned int opn, rword offset);
    llvm::MCInst reloc(ExecBlock *execBlock, CPUMode cpuMode);
};

class EpilogueRel : public RelocatableInst, public AutoAlloc<RelocatableInst, EpilogueRel> {
    unsigned int opn;
    rword        offset;

  public:
    EpilogueRel(llvm::MCInst inst, unsigned int opn, rword offset);
    llvm::MCInst reloc(ExecBlock *execBlock, CPUMode cpuMode);
};


class HostPCRel : public RelocatableInst, public AutoAlloc<RelocatableInst, HostPCRel> {
    unsigned int opn;
    rword        offset;

  public:
    HostPCRel(llvm::MCInst inst, unsigned int opn, rword offset);
    llvm::MCInst reloc(ExecBlock *execBlock, CPUMode cpuMode);
};

class InstId : public RelocatableInst, public AutoAlloc<RelocatableInst, InstId> {
    unsigned int opn;

  public:
    InstId(llvm::MCInst inst, unsigned int opn);
    llvm::MCInst reloc(ExecBlock *execBlock, CPUMode cpuMode);
};

class AdjustPCAlign : public RelocatableInst, public AutoAlloc<RelocatableInst, AdjustPCAlign> {
    unsigned int opn;

  public:
    AdjustPCAlign(llvm::MCInst inst, unsigned int opn);
    llvm::MCInst reloc(ExecBlock *execBlock, CPUMode cpuMode);
};



}

#endif
