
#include "llvm/Pass.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <string>

#include "VirtineCompiler.h"


#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"


// stolen from my kernel's userspace :^)
#include "command.h"

void debug_hexdump(void *vbuf, size_t len);


#define FMT(n, ...)                \
  ({                               \
    char buf[n];                   \
    snprintf(buf, n, __VA_ARGS__); \
    std::string b(buf);            \
    b;                             \
  })



namespace {
  struct VirtinePass : public llvm::FunctionPass {
    static char ID;
    VirtinePass() : llvm::FunctionPass(ID) {}

    bool runOnFunction(llvm::Function &fn) override {
      // llvm::errs() << *fn.getParent() << "\n";
      auto section = fn.getSection();
      if (section.startswith("$__VIRTINE__")) {
        // printf("mod: %s function %s is a virtine in %s.\n", fn.getParent()->getName().data(), fn.getName().data(), section.data());

        auto fname = fn.getName().data();
        auto &ctx = fn.getContext();
        auto *mod = fn.getParent();

        auto vc = VirtineCompiler(fn);
        vc.run_analysis();
        auto code = vc.compile();



        // debug_hexdump(code.data(), code.size());

        auto arg_struct_type = VirtineCompiler::create_argument_struct(fn, true);

        // first, we gotta remove the old body from the function
        fn.deleteBody();

        auto bb = llvm::BasicBlock::Create(ctx, "entry", &fn);
        llvm::IRBuilder<> builder(ctx);
        builder.SetInsertPoint(bb);


        auto argptr = llvm::PointerType::get(arg_struct_type, 0);

        auto args = builder.CreateAlloca(arg_struct_type);
        args->setName("argument_struct");

        int i = 0;
        for (auto &arg : fn.args()) {
          std::vector<llvm::Value *> indices(2);
          indices[0] = llvm::ConstantInt::get(ctx, llvm::APInt(32, 0, true));
          indices[1] = llvm::ConstantInt::get(ctx, llvm::APInt(32, i++, true));
          auto ptr = builder.CreateGEP(arg_struct_type, args, indices);
          if (arg.getType()->isPointerTy()) {
            builder.CreateStore(builder.CreateLoad(&arg), ptr);
          } else {
            builder.CreateStore(&arg, ptr);
          }
        }


        /*
         * now we need to get the wasp library function to call
         */
        auto chartype = llvm::IntegerType::get(ctx, 8);
        auto charptr_type = llvm::PointerType::get(chartype, 0);
        auto size_t_type = llvm::IntegerType::get(ctx, 64);

        llvm::Function *wasp_run_func = mod->getFunction("wasp_run_virtine");

        // if it's null, we gotta make it.
        if (wasp_run_func == nullptr) {
          std::vector<llvm::Type *> args;

          args.push_back(charptr_type);  // char *code
          args.push_back(size_t_type);   // size_t codesz
          args.push_back(size_t_type);   // size_t memsz
          args.push_back(charptr_type);  // void *arg
          args.push_back(size_t_type);   // size_t argsz

          args.push_back(charptr_type);  // void *config (a struct virtine_config)
          auto wasp_run_type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), args, false);


          wasp_run_func = llvm::Function::Create(wasp_run_type, llvm::Function::ExternalLinkage, "wasp_run_virtine", mod);
        }

        auto *codearray_ty = llvm::ArrayType::get(chartype, code.size());
        auto *code_global = new llvm::GlobalVariable(*mod, codearray_ty, false, llvm::GlobalValue::PrivateLinkage,
            0,  // has initializer, specified below
            FMT(50, "__virtine_%s_code", fn.getName().data()));


        std::vector<llvm::Constant *> data;
        for (uint8_t byte : code)
          data.push_back(llvm::ConstantInt::get(chartype, byte, false));

        llvm::Constant *init = llvm::ConstantArray::get(codearray_ty, data);
        code_global->setInitializer(init);


        // now we call the virtine runtime code...
        {
          std::vector<llvm::Value *> argv;

          // char *code
          argv.push_back(code_global);
          // size_t code_size
          argv.push_back(llvm::ConstantInt::get(size_t_type, code.size()));

          size_t ram_size = (4096 * 128);
          // memory size 2 mb plus the size of the code.
          argv.push_back(llvm::ConstantInt::get(size_t_type, ram_size));



          // argument structure
          argv.push_back(args);

          // argument size is a little hard to get. Basically, we calculate it
          // by getting a pointer to an arg type at NULL (0), then offsetting
          // to index 1, and casting that value to an int. This effectively
          // means we have the address of the second struct in memory (the size
          // of the first)
          auto NULL_PTR = llvm::ConstantPointerNull::get(llvm::PointerType::get(arg_struct_type, 0));
          auto size_ptr = builder.CreateGEP(arg_struct_type, NULL_PTR, llvm::ConstantInt::get(ctx, llvm::APInt(32, 1, true)), "size_hack");
          auto size = builder.CreatePtrToInt(size_ptr, size_t_type, "size");
          argv.push_back(size);


          llvm::Value *config = llvm::ConstantPointerNull::get(charptr_type);

          if (section.startswith("$__VIRTINE__cfg=")) {
            // printf("this one has a config...\n");
            auto s = section.slice(strlen("$__VIRTINE__cfg="), section.size());
            // printf("s: %s\n", s.data());
            config = fn.getParent()->getNamedGlobal(s);
          }

          argv.push_back(config);

          builder.CreateCall(wasp_run_func, argv);
        }


        i = 0;
        for (auto &arg : fn.args()) {
          int ind = i++;
          if (arg.getType()->isPointerTy()) {
            // printf("need to restore %d\n", ind);
            std::vector<llvm::Value *> indices(2);
            indices[0] = llvm::ConstantInt::get(ctx, llvm::APInt(32, 0, true));
            indices[1] = llvm::ConstantInt::get(ctx, llvm::APInt(32, ind, true));
            auto ptr = builder.CreateGEP(arg_struct_type, args, indices);
            auto val = builder.CreateLoad(ptr);
            builder.CreateStore(val, &arg);
          }
        }



        llvm::Value *return_value = NULL;
        // index into the end of the argument array
        {
          std::vector<llvm::Value *> indices(2);
          indices[0] = llvm::ConstantInt::get(ctx, llvm::APInt(32, 0, true));
          indices[1] = llvm::ConstantInt::get(ctx, llvm::APInt(32, fn.arg_size(), true));
          auto ptr = builder.CreateGEP(arg_struct_type, args, indices);

          return_value = builder.CreateLoad(ptr);
        }

        builder.CreateRet(return_value);

        // clear the section so the linker can do what it wants...
        fn.setSection("");

				/*
        printf("===================================================================\n");
        fn.print(llvm::errs(), NULL);
        printf("===================================================================\n");
				*/

        return true;


      } else {
      }

      return false;
    }
  };  // end of struct VirtinePass
}  // end of anonymous namespace

char VirtinePass::ID = 0;
static llvm::RegisterPass<VirtinePass> X("virtine", "Virtine Compiler Pass", false /* Only looks at CFG */, false /* Analysis Pass */);

static llvm::RegisterStandardPasses Y(
    llvm::PassManagerBuilder::EP_EarlyAsPossible, [](const llvm::PassManagerBuilder &Builder, llvm::legacy::PassManagerBase &PM) {
      PM.add(new VirtinePass());
    });



#define C_RED 91
#define C_GREEN 92
#define C_YELLOW 93
#define C_BLUE 94
#define C_MAGENTA 95
#define C_CYAN 96

#define C_RESET 0
#define C_GRAY 90

static void set_color(int code) {
  static int current_color = 0;
  if (code != current_color) {
    printf("\x1b[%dm", code);
    current_color = code;
  }
}

static void set_color_for(char c) {
  if (c >= 'A' && c <= 'z') {
    set_color(C_YELLOW);
  } else if (c >= '!' && c <= '~') {
    set_color(C_CYAN);
  } else if (c == '\n' || c == '\r') {
    set_color(C_GREEN);
  } else if (c == '\a' || c == '\b' || c == 0x1b || c == '\f' || c == '\n' || c == '\r') {
    set_color(C_RED);
  } else if ((unsigned char)c == 0xFF) {
    set_color(C_MAGENTA);
  } else {
    set_color(C_GRAY);
  }
}

void debug_hexdump(void *vbuf, size_t len) {
  unsigned awidth = 4;

  if (len > 0xFFFFL) awidth = 8;

  unsigned char *buf = (unsigned char *)vbuf;
  int w = 16;

  // array of valid address checks
  char valid[16];

  int has_validated = 0;
  off_t last_validated_page = 0;
  int is_valid = 0;

  for (unsigned long long i = 0; i < len; i += w) {
    unsigned char *line = buf + i;


    for (int c = 0; c < w; c++) {
      off_t page = (off_t)(line + c) >> 12;

      if (!has_validated || page != last_validated_page) {
        is_valid = 1;
        has_validated = 1;
      }

      valid[c] = is_valid;
      last_validated_page = page;
    }

    set_color(C_RESET);
    printf("|");
    set_color(C_GRAY);

    printf("%.*llx", awidth, i);

    set_color(C_RESET);
    printf("|");
    for (int c = 0; c < w; c++) {
      if (c % 8 == 0) {
        printf(" ");
      }

      if (valid[c] == 0) {
        set_color(C_RED);
        printf("?? ");
        continue;
      }

      if (i + c >= len) {
        printf("   ");
      } else {
        set_color_for(line[c]);
        printf("%02X ", line[c]);
      }
    }

    set_color(C_RESET);
    printf("|");
    for (int c = 0; c < w; c++) {
      if (c != 0 && (c % 8 == 0)) {
        set_color(C_RESET);
        printf(" ");
      }


      if (valid[c] == 0) {
        set_color(C_RED);
        printf("?");
        continue;
      }

      if (i + c >= len) {
        printf(" ");
      } else {
        set_color_for(line[c]);
        printf("%c", (line[c] < 0x20) || (line[c] > 0x7e) ? '.' : line[c]);
      }
    }
    set_color(C_RESET);
    printf("|\n");
  }
}
