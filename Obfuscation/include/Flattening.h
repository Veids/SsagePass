#ifndef LLVM_FLATTENING_H
#define LLVM_FLATTENING_H
// LLVM libs
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Utils/LowerSwitch.h"
// Libs
#include "Utils.h"
#include "CryptoUtils.h"
// System libs
#include <vector>
#include <cstdlib>
#include <ctime>
namespace llvm{
    class FlatteningPass : public PassInfoMixin<FlatteningPass>{ 
        public:
            bool flag;
            FlatteningPass(bool flag){
                this->flag = flag;
            } // 携带flag的构造函数
            PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
            PreservedAnalyses flatten(Function *F);
    };
    FlatteningPass *createFlattening(bool flag);
}
#endif // LLVM_FLATTENING_H
