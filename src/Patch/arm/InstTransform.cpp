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
#include "Patch/arm/InstTransform.h"
namespace QBDI {

ThumbLDRpciTransform::ThumbLDRpciTransform(Temp tmp) :
  temp(tmp)
{}

void ThumbLDRpciTransform::transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) {
  llvm::MCInst newInst;
  newInst.setOpcode(llvm::ARM::t2LDRi12);
  newInst.addOperand(inst.getOperand(0));
  newInst.addOperand(llvm::MCOperand::createReg(temp_manager->getRegForTemp(this->temp)));
  newInst.addOperand(inst.getOperand(1));
  newInst.addOperand(llvm::MCOperand::createImm(14));
  newInst.addOperand(llvm::MCOperand::createReg(0));
  inst = std::move(newInst);

}
}
