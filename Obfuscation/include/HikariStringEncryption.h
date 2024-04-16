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
#include <iostream>
#include <vector>

using namespace std;
namespace llvm {
    struct HikariStringEncryptionPass : public PassInfoMixin<HikariStringEncryptionPass> {
        public:
            bool flag;
            bool appleptrauth;
            bool opaquepointers;
            std::map<Function * /*Function*/, GlobalVariable * /*Decryption Status*/>
                encstatus;
            std::map<GlobalVariable *, std::pair<Constant *, GlobalVariable *>> mgv2keys;
            std::map<Constant *, SmallVector<unsigned int, 16>> unencryptedindex;
            vector<GlobalVariable *> genedgv;

            HikariStringEncryptionPass(bool flag){this->flag = flag;}
            PreservedAnalyses run(Module &M, ModuleAnalysisManager& AM);

        private:
            bool handleableGV(GlobalVariable *GV);
            void HandleFunction(Function *Func);

            GlobalVariable *ObjectiveCString(GlobalVariable *GV, std::string name,
                    GlobalVariable *newString,
                    ConstantStruct *CS);

            void HandleDecryptionBlock(
                    BasicBlock *B, BasicBlock *C,
                    std::map<GlobalVariable *, std::pair<Constant *, GlobalVariable *>>
                    &GV2Keys);

            void processStructMembers(ConstantStruct *,
                    std::vector<GlobalVariable *> *,
                    std::vector<GlobalVariable *> *,
                    std::set<User *> *, bool *);

            void processArrayMembers(ConstantArray *CA,
                    std::vector<GlobalVariable *> *,
                    std::vector<GlobalVariable *> *,
                    std::set<User *> *, bool *);
    };
    HikariStringEncryptionPass *createHikariStringEncryption(bool flag); // 创建字符串加密
}
#endif
