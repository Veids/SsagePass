// LLVM libs
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
// User libs
#include "ObfuscationOptions.h"
#include "CryptoUtils.h"
#include "Utils.h"
// System libs
#include <random>

namespace llvm{
    class IndirectCallPass : public PassInfoMixin<IndirectCallPass>{ 
        public:
            static char ID;
            bool flag;
            unsigned pointerSize;

            ObfuscationOptions *Options;
            std::map<Function *, unsigned> CalleeNumbering;
            std::vector<CallInst *> CallSites;
            std::vector<Function *> Callees;
            CryptoUtils RandomEngine;

            IndirectCallPass(bool flag){
                this->flag = flag;
            }

            void NumberCallees(Function &F);
            GlobalVariable *getIndirectCallees(Function &F, ConstantInt *EncKey);
            PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
            bool doIndirctCall(Function &F);
    };
}
