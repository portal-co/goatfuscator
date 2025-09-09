#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Transforms/Utils/Local.h"

#include "llvm/IR/Instructions.h"

#include "pstream.h"
#include "DuplicateBB.h"
#include "Util.h"
#include <list>
#include <llvm-17/llvm/IR/GlobalValue.h>
#include <openssl/pem.h>

#include "llvm/IRReader/IRReader.h"
#include <cstdint>
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassPlugin.h>
#include <random>
#include <string>
#include <vector>
namespace{
  
}
std::map<const llvm::Target*, std::shared_ptr<ObfuscationTarget>> obf_targets = {

};
void clearModule(llvm::Module *m){
  std::vector<llvm::GlobalValue*> vs;
  for(auto &v: m->global_values())vs.push_back(&v);
  for(auto *v: vs)v->removeFromParent();
}

bool EncryptString(const std::vector<uint8_t>& InStr /*plaintext*/, EVP_PKEY* InPublicKey /*path to public key pem file*/, std::vector<uint8_t>& OutString /*ciphertext*/) {
    
    // Load key
    // FILE* f = fopen(InPublicKey.c_str(), "r");
    // EVP_PKEY* pkey = PEM_read_PUBKEY(f, NULL, NULL, NULL);
    // fclose(f);
    
    // Create/initialize context
    EVP_PKEY_CTX* ctx;
    ctx = EVP_PKEY_CTX_new(InPublicKey, NULL);
    EVP_PKEY_encrypt_init(ctx);

    // Specify padding: default is PKCS#1 v1.5
    // EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING); // for OAEP with SHA1 for both digests

    // Encryption
    size_t ciphertextLen;
    EVP_PKEY_encrypt(ctx, NULL, &ciphertextLen, (const unsigned char*)&InStr[0], InStr.size());
    unsigned char* ciphertext = (unsigned char*)OPENSSL_malloc(ciphertextLen);
    EVP_PKEY_encrypt(ctx, ciphertext, &ciphertextLen, (const unsigned char*)&InStr[0], InStr.size());
    OutString = std::vector(ciphertext,ciphertext + ciphertextLen);

    // Release memory
    // EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    OPENSSL_free(ciphertext);

    return true; // add exception/error handling
}

using namespace llvm;
void unCringify(llvm::Function *f) {
  bool hk = f->getName() == "HikariIndirectBranchTargetWrapper";
  llvm::Function *hkd = f;
  for (auto &b : *f) {
    for (auto &i : b) {
      llvm::CallInst *d = dyn_cast<llvm::CallInst>(&i);
      if (d) {
        llvm::Function *f = d->getCalledFunction();
        if (f) {
          auto n = f->getName();
          // if (n == "HikariIndirectBranchTargetWrapper"){
          //   hk = true; // Cringe
          //   hkd = f;
          // }
        }
      }
    }
  }
  if(hk){
    
  }
}

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
  std::vector<PHINode *> tmpPhi;
  std::vector<Instruction *> tmpReg;
  BasicBlock *bbEntry = &*f->begin();

  do {
    tmpPhi.clear();
    tmpReg.clear();

    for (Function::iterator i = f->begin(); i != f->end(); ++i) {

      for (BasicBlock::iterator j = i->begin(); j != i->end(); ++j) {

        if (isa<PHINode>(j)) {
          PHINode *phi = cast<PHINode>(j);
          tmpPhi.push_back(phi);
          continue;
        }
        if (!(isa<AllocaInst>(j) && j->getParent() == bbEntry) &&
            (valueEscapes(&*j) || j->isUsedOutsideOfBlock(&*i))) {
          tmpReg.push_back(&*j);
          continue;
        }
      }
    }
    for (unsigned int i = 0; i != tmpReg.size(); ++i) {
      DemoteRegToStack(*tmpReg.at(i), f->begin()->getTerminator());
    }

    for (unsigned int i = 0; i != tmpPhi.size(); ++i) {
      DemotePHIToStack(tmpPhi.at(i), f->begin()->getTerminator());
    }

  } while (tmpReg.size() != 0 || tmpPhi.size() != 0);
}

namespace {
std::string newGarbage(int sample = 16) {
  std::random_device rd;
  std::mt19937 g(rd());
  std::uniform_int_distribution<uint32_t> rand(0, UINT32_MAX);
  std::string s = "";
  // std::string junk[] = {"leaq	-4(%rbp), %rdx\n", "xorq %r11, %rsp\n",
  //                       "callq *(%rbx)\n", "popfq\n", "movq %rbp, %rsp\n"};
  // if (is64) {
  //   for (int i = 0; i < 10; i++) {
  //     uint32_t j = rand(g) % 15;
  //     if (j < 5) {
  //       s += junk[j];
  //     }
  //   }
  //   if (rand(g) % 5 == 0) {
  //     s += "addq " + std::to_string(4 * (rand(g) % 20)) + ", %rsp\n";
  //     s += "popq %rbp\nretq\n";
  //   }
  // }
  // uint8_t onebyte[] = {0xEB, 0xE9, 0xE8, 0xE3, 0xE2, 0xE1, 0xE0, 0x70, 0x71,
  //                      0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
  //                      0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0xC2, 0xCA, 0xFF, 0x0F};
  while (rand(g) % sample != 0)
    s += ".byte " + std::to_string(rand(g) % 256) + ";\n";
  return s;
}
} // namespace

InlineAsm *generateGarbage(Function *f) {
  bool isWA =
      Triple(f->getParent()->getTargetTriple()).getArch() == Triple::wasm32;
  isWA = isWA || (Triple(f->getParent()->getTargetTriple()).getArch() ==
                  Triple::wasm64);
  if (isWA) {
    return InlineAsm::get(
        FunctionType::get(Type::getVoidTy(f->getContext()), false), "", "",
        true, false);
  }
  auto s = newGarbage();
  InlineAsm *IA =
      InlineAsm::get(FunctionType::get(Type::getVoidTy(f->getContext()), false),
                     s, "", true, false);
  return IA;
}

InlineAsm *generateNull(Function *f) {
  bool isWA =
      Triple(f->getParent()->getTargetTriple()).getArch() == Triple::wasm32;
  isWA = isWA || (Triple(f->getParent()->getTargetTriple()).getArch() ==
                  Triple::wasm64);
  if (isWA) {
    return InlineAsm::get(
        FunctionType::get(Type::getVoidTy(f->getContext()), false), "", "",
        true, false);
  }
  std::random_device rd;
  std::mt19937_64 g(rd());
  std::string s = "";
  while (g() % 8 != 0) {
    uint64_t lid = g();
    std::string lt = std::to_string(lid);
    for(char &c: lt)c += 'a' - '0';
    if (Triple(f->getParent()->getTargetTriple()).isX86()) {
      switch (g() % 2) {
      case 0:
        s += "push %rax;lea 1f(%rip), %rax;push %rax;ret;\n";
        s += newGarbage(8);
        // s += "\nd";
        // s += std::to_string(lid);
        // s += ":\n.pushsection .data\n.quad ";
        // s += "l";
        // s += std::to_string(lid);
        s += "1:\npop %rax\n\n";
        break;
      case 1:
        s += "push %rax;call 1f;\n";
        s += newGarbage(8);
        // s += "\n.set d";
        // s += lt;
        // s += ", 8";
        s += "1:\npop %rax\npop %rax\n";
        break;
      };
    };
  };
  InlineAsm *IA =
      InlineAsm::get(FunctionType::get(Type::getVoidTy(f->getContext()), false),
                     s, "", true, false);
  return IA;
}

uint32_t fnvHash(const uint32_t data, uint32_t b) {
  b = (b ^ ((data >> 0) & 0xFF)) * fnvPrime;
  b = (b ^ ((data >> 8) & 0xFF)) * fnvPrime;
  b = (b ^ ((data >> 16) & 0xFF)) * fnvPrime;
  b = (b ^ ((data >> 24) & 0xFF)) * fnvPrime;
  return b;
}

uint64_t powerMod(uint32_t a, uint32_t n, uint32_t mod) {
  uint64_t power = a, result = 1;

  while (n) {
    if (n & 1)
      result = (result * power) % mod;
    power = (power * power) % mod;
    n >>= 1;
  }
  return result;
}

bool witness(uint32_t a, uint32_t n) {
  uint32_t t, u, i;
  uint64_t prev, curr;

  u = n / 2;
  t = 1;
  while (!(u & 1)) {
    u /= 2;
    ++t;
  }

  prev = powerMod(a, u, n);
  for (i = 1; i <= t; ++i) {
    curr = (prev * prev) % n;
    if ((curr == 1) && (prev != 1) && (prev != n - 1))
      return true;
    prev = curr;
  }
  if (curr != 1)
    return true;
  return false;
}

bool isPrime(uint32_t number) {
  if (((!(number & 1)) && number != 2) || (number < 2) ||
      (number % 3 == 0 && number != 3))
    return (false);

  if (number < 1373653) {
    for (uint32_t k = 1; 36 * k * k - 12 * k < number; ++k)
      if ((number % (6 * k + 1) == 0) || (number % (6 * k - 1) == 0))
        return (false);
    return true;
  }

  if (number < 9080191) {
    if (witness(31, number))
      return false;
    if (witness(73, number))
      return false;
    return true;
  }

  if (witness(2, number))
    return false;
  if (witness(7, number))
    return false;
  if (witness(61, number))
    return false;
  return true;
}

uint32_t randPrime(uint32_t min, uint32_t max) {
  std::random_device rd;
  std::mt19937 g(rd());
  std::uniform_int_distribution<uint32_t> rand(min, max);
  uint32_t p = rand(g);
  while (!isPrime(p)) {
    p = rand(g);
  }
  return p;
}

uint64_t modinv(uint64_t a) {
  uint64_t x = a;
  for (int k = 2; k < 64; k *= 2) {
    x = (x * (2 - a * x)) % (1ULL << k);
  }
  return x * (2 - a * x);
}

uint32_t xr() {
  std::random_device rd;
  std::mt19937 g(rd());
  return g();
}

void link(Module &m, std::string code) {
  std::random_device rd;
  std::string path = "/tmp/goatf-";
  path += std::to_string(rd());
  redi::pstream p(std::string("$CXX -c --emit-llvm -o ") + path + " - ");
  p << code;
  char c;
  while (p >> c)
    std::cerr << c;
  SMDiagnostic err;
  auto x = llvm::parseIRFile(path, err, m.getContext());
  llvm::Linker::linkModules(m, std::move(x));
}

void demPhis(llvm::Function &F){
  std::list<llvm::PHINode*> WorkList;
  for (BasicBlock &BB : F)
    for (auto &Phi : BB.phis())
      WorkList.push_front(&Phi);
 
  // Demote phi nodes
  // NumPhisDemoted += WorkList.size();
  for (auto I : WorkList)
    DemotePHIToStack(I);
}

llvm::PassPluginLibraryInfo getDuplicateBBPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "goatfuscator", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            addRIV(PB);
            PB.registerPipelineParsingCallback(
                [](StringRef Name, PassManager<Function> &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "duplicate-bb") {
                    FPM.addPass(DuplicateBB());
                    return true;
                  }
                  if (Name == "bb2func") {
                    addBB2Func(FPM);
                    return true;
                  }
                  // if (Name == "flattening") {
                  //   addFlattening(FPM);
                  //   return true;
                  // }
                  if (Name == "connect") {
                    addConnect(FPM);
                    return true;
                  }
                  if (Name == "obfCon") {
                    addObfCon(FPM);
                    return true;
                  };
                  return false;
                });
            PB.registerPipelineParsingCallback(
                [](StringRef Name, PassManager<Module> &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "merge") {
                    addMerge(FPM);
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getDuplicateBBPluginInfo();
}