#ifndef LLVM_HIKARI_STRING_ENCRYPTION_H
#define LLVM_HIKARI_STRING_ENCRYPTION_H
// LLVM libs
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
// User libs
#include "CryptoUtils.h"
#include "Utils.h"
// System libs
#include <map>
#include <set>
#include <unordered_set>
#include <iostream>
#include <vector>

using namespace std;
namespace llvm {
    struct HikariStringEncryptionPass : public PassInfoMixin<HikariStringEncryptionPass> {
        public:
            bool flag;
            bool appleptrauth;
            bool opaquepointers;
            std::unordered_map<Function * /*Function*/, GlobalVariable * /*Decryption Status*/>
                encstatus;
            std::unordered_map<GlobalVariable *, std::pair<Constant *, GlobalVariable *>> mgv2keys;
            std::unordered_map<Constant *, SmallVector<unsigned int, 16>> unencryptedindex;
            SmallVector<GlobalVariable *, 32> genedgv;

            HikariStringEncryptionPass(bool flag){this->flag = flag;}
            PreservedAnalyses run(Module &M, ModuleAnalysisManager& AM);

        private:
            bool handleableGV(GlobalVariable *GV);

            void HandleUser(User *U, SmallVector<GlobalVariable *, 32> &Globals,
                    std::set<User *> &Users,
                    std::unordered_set<User *> &VisitedUsers);

            void HandleFunction(Function *Func);

            GlobalVariable *ObjectiveCString(GlobalVariable *GV, std::string name,
                    GlobalVariable *newString,
                    ConstantStruct *CS);

            void HandleDecryptionBlock(
                    BasicBlock *B, BasicBlock *C,
                    std::unordered_map<GlobalVariable *, std::pair<Constant *, GlobalVariable *>>
                    &GV2Keys);

            void processConstantAggregate(
                    GlobalVariable *strGV, ConstantAggregate *CA,
                    std::set<GlobalVariable *> *rawStrings,
                    SmallVector<GlobalVariable *, 32> *unhandleablegvs,
                    SmallVector<GlobalVariable *, 32> *Globals,
                    std::set<User *> *Users, bool *breakFor);
    };
    HikariStringEncryptionPass *createHikariStringEncryption(bool flag); // 创建字符串加密
}
#endif
