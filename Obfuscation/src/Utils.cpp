//https://github.com/61bcdefg/Hikari-LLVM15-Core/blob/0ba1c7bdfac5c8f41623ea48bcc7fc3b1b40f0d9/Utils.cpp
/**
 * @file Utils.cpp
 * @author SsageParuders
 * @brief 本代码参考原OLLVM项目:https://github.com/obfuscator-llvm/obfuscator
 *        感谢地球人前辈的指点
 * @version 0.1
 * @date 2022-07-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Utils.h"

using namespace llvm;
using std::vector;

LLVMContext *CONTEXT = nullptr;

// Shamefully borrowed from ../Scalar/RegToMem.cpp :(
bool valueEscapes(Instruction *Inst) {
  BasicBlock *BB = Inst->getParent();
  for (Value::use_iterator UI = Inst->use_begin(), E = Inst->use_end(); UI != E;
       ++UI) {
    Instruction *I = cast<Instruction>(*UI);
    if (I->getParent() != BB || isa<PHINode>(I)) {
      return true;
    }
  }
  return false;
}

void fixStack(Function *f) {
  // Try to remove phi node and demote reg to stack
  SmallVector<PHINode *, 8> tmpPhi;
  SmallVector<Instruction *, 32> tmpReg;
  BasicBlock *bbEntry = &*f->begin();
  // Find first non-alloca instruction and create insertion point. This is
  // safe if block is well-formed: it always have terminator, otherwise
  // we'll get and assertion.
  BasicBlock::iterator I = bbEntry->begin();
  while (isa<AllocaInst>(I))
    ++I;
  Instruction *AllocaInsertionPoint = &*I;
  do {
    tmpPhi.clear();
    tmpReg.clear();
    for (BasicBlock &i : *f) {
      for (Instruction &j : i) {
        if (isa<PHINode>(&j)) {
          PHINode *phi = cast<PHINode>(&j);
          tmpPhi.emplace_back(phi);
          continue;
        }
        if (!(isa<AllocaInst>(&j) && j.getParent() == bbEntry) &&
            (valueEscapes(&j) || j.isUsedOutsideOfBlock(&i))) {
          tmpReg.emplace_back(&j);
          continue;
        }
      }
    }
    for (Instruction *I : tmpReg)
      DemoteRegToStack(*I, false, AllocaInsertionPoint);
    for (PHINode *P : tmpPhi)
      DemotePHIToStack(P, AllocaInsertionPoint);
  } while (tmpReg.size() != 0 || tmpPhi.size() != 0);
}

// Unlike O-LLVM which uses __attribute__ that is not supported by the ObjC
// CFE. We use a dummy call here and remove the call later Very dumb and
// definitely slower than the function attribute method Merely a hack
bool readFlag(Function *f, std::string attribute) {
  for (Instruction &I : instructions(f)) {
    Instruction *Inst = &I;
    if (CallInst *CI = dyn_cast<CallInst>(Inst)) {
      if (CI->getCalledFunction() != nullptr &&
          CI->getCalledFunction()->getName().contains("hikari_" + attribute)) {
        CI->eraseFromParent();
        return true;
      }
    }
  }
  return false;
}

bool llvm::toObfuscate(bool flag, Function *f, std::string const &attribute) { //取自原版ollvm项目
    // Check if declaration and external linkage
    if (f->isDeclaration() || f->hasAvailableExternallyLinkage()) {
        return false;
    }
    std::string attr = attribute;
    std::string attrNo = "no" + attr;
    if (readAnnotationMetadata(f, attrNo) || readFlag(f, attrNo)) {
        return false;
    }
    if (readAnnotationMetadata(f, attr) || readFlag(f, attr)) {
        return true;
    }
    return flag;
}

bool llvm::toObfuscateBoolOption(Function *f, std::string option, bool *val) {
  std::string opt = option;
  std::string optDisable = "no" + option;
  if (readAnnotationMetadata(f, optDisable) || readFlag(f, optDisable)) {
    *val = false;
    return true;
  }
  if (readAnnotationMetadata(f, opt) || readFlag(f, opt)) {
    *val = true;
    return true;
  }
  return false;
}

bool llvm::readAnnotationMetadataUint32OptVal(Function *f, std::string opt,
                                        uint32_t *val) {
  MDNode *Existing = f->getMetadata(llvm::obfkindid);
  if (Existing) {
    MDTuple *Tuple = cast<MDTuple>(Existing);
    for (auto &N : Tuple->operands()) {
      StringRef mdstr = cast<MDString>(N.get())->getString();
      std::string estr = opt + "=";
      if (mdstr.startswith(estr)) {
        *val = atoi(mdstr.substr(strlen(estr.c_str())).str().c_str());
        return true;
      }
    }
  }
  return false;
}

bool llvm::readFlagUint32OptVal(Function *f, std::string opt, uint32_t *val) {
  for (Instruction &I : instructions(f)) {
    Instruction *Inst = &I;
    if (CallInst *CI = dyn_cast<CallInst>(Inst)) {
      if (CI->getCalledFunction() != nullptr &&
          CI->getCalledFunction()->getName().contains("hikari_" + opt)) {
        if (ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0))) {
          *val = (uint32_t)C->getValue().getZExtValue();
          CI->eraseFromParent();
          return true;
        }
      }
    }
  }
  return false;
}

bool llvm::toObfuscateUint32Option(Function *f, std::string option, uint32_t *val) {
  if (llvm::readAnnotationMetadataUint32OptVal(f, option, val) ||
      llvm::readFlagUint32OptVal(f, option, val))
    return true;
  return false;
}

bool llvm::hasApplePtrauth(Module *M) {
  for (GlobalVariable &GV : M->globals())
    if (GV.getSection() == "llvm.ptrauth")
      return true;
  return false;
}

void llvm::FixBasicBlockConstantExpr(BasicBlock *BB) {
  // Replace ConstantExpr with equal instructions
  // Otherwise replacing on Constant will crash the compiler
  // Things to note:
  // - Phis must be placed at BB start so CEs must be placed prior to current BB
  assert(!BB->empty() && "BasicBlock is empty!");
  assert((BB->getParent() != NULL) && "BasicBlock must be in a Function!");
  Instruction *FunctionInsertPt = &*(BB->getParent()->getEntryBlock().getFirstInsertionPt());
  // Instruction* LocalBBInsertPt=&*(BB.getFirstInsertionPt());
  for (Instruction &I : *BB) {
    if (isa<LandingPadInst>(I) || isa<FuncletPadInst>(I)) {
        continue;
    }
    for (unsigned i = 0; i < I.getNumOperands(); i++) {
      if (ConstantExpr *C = dyn_cast<ConstantExpr>(I.getOperand(i))) {
        Instruction *InsertPt = &I;
        IRBuilder<NoFolder> IRB(InsertPt);
        if (isa<PHINode>(I)) {
          IRB.SetInsertPoint(FunctionInsertPt);
        }
        Instruction *Inst = IRB.Insert(C->getAsInstruction());
        I.setOperand(i, Inst);
      }
    }
  }
}

void llvm::FixFunctionConstantExpr(Function *Func) {
  // Replace ConstantExpr with equal instructions
  // Otherwise replacing on Constant will crash the compiler
  for (BasicBlock &BB : *Func) {
    FixBasicBlockConstantExpr(&BB);
  }
}

void llvm::turnOffOptimization(Function *f) {
  f->removeFnAttr(Attribute::AttrKind::MinSize);
  f->removeFnAttr(Attribute::AttrKind::OptimizeForSize);
  if (!f->hasFnAttribute(Attribute::AttrKind::OptimizeNone) &&
      !f->hasFnAttribute(Attribute::AttrKind::AlwaysInline)) {
    f->addFnAttr(Attribute::AttrKind::OptimizeNone);
    f->addFnAttr(Attribute::AttrKind::NoInline);
  }
}

bool llvm::readAnnotationMetadata(Function *f, std::string annotation) {
  MDNode *Existing = f->getMetadata(obfkindid);
  if (Existing) {
    MDTuple *Tuple = cast<MDTuple>(Existing);
    for (auto &N : Tuple->operands())
      if (cast<MDString>(N.get())->getString() == annotation)
        return true;
  }
  return false;
}

void llvm::writeAnnotationMetadata(Function *f, std::string annotation) {
  LLVMContext &Context = f->getContext();
  MDBuilder MDB(Context);

  MDNode *Existing = f->getMetadata(obfkindid);
  SmallVector<Metadata *, 4> Names;
  bool AppendName = true;
  if (Existing) {
    MDTuple *Tuple = cast<MDTuple>(Existing);
    for (auto &N : Tuple->operands()) {
      if (cast<MDString>(N.get())->getString() == annotation)
        AppendName = false;
      Names.emplace_back(N.get());
    }
  }
  if (AppendName)
    Names.emplace_back(MDB.createString(annotation));

  MDNode *MD = MDTuple::get(Context, Names);
  f->setMetadata(obfkindid, MD);
}

bool AreUsersInOneFunction(GlobalVariable *GV) {
  SmallPtrSet<Function *, 6> userFunctions;
  for (User *U : GV->users()) {
    if (Instruction *I = dyn_cast<Instruction>(U)) {
      userFunctions.insert(I->getFunction());
    } else if (ConstantExpr *CE = dyn_cast<ConstantExpr>(U)) {
      for (User *U2 : CE->users()) {
        if (Instruction *I = dyn_cast<Instruction>(U2)) {
          userFunctions.insert(I->getFunction());
        }
      }
    } else {
      return false;
    }
  }
  return userFunctions.size() <= 1;
}
