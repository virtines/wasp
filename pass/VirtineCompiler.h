#pragma once


#include <stdint.h>
#include <vector>

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

class VirtineCompiler {
  std::unique_ptr<llvm::Module> m_mod;
  llvm::Function *m_func;

  llvm::StructType *m_arg_struct;

 public:
  VirtineCompiler(llvm::Function &source);

  struct Analysis {
    // the largest bit width for each of these types.
    // This is so we can possibly change the bootloader
    // based on the requirements of the virtine
    int largest_int;
    int largest_float;
  };
  Analysis run_analysis(void);

  // compile into the byte array
  std::vector<uint8_t> compile();

  /*
   * given a function, construct an argument structure in it's module
   */
  static llvm::StructType *create_argument_struct(llvm::Function &, bool surrogate = false);

 protected:
  // run a single dead-code pass rooted at a certain function
  bool single_dead_code_pass(llvm::Function &root);


  llvm::Function *create_main(llvm::Function &);


  auto &mod() { return *m_mod; }
  auto &func() { return *m_func; }
  auto &arg_struct(void) { return *m_arg_struct; }
};
