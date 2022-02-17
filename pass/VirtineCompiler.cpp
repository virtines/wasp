#include "VirtineCompiler.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include "command.h"


#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/TypeFinder.h"

static auto copy_module(const llvm::Module &mod) {
  llvm::SmallString<0> BC;
  llvm::raw_svector_ostream BCOS(BC);
  llvm::WriteBitcodeToFile(mod, BCOS);

  return std::move(
      llvm::parseBitcodeFile(llvm::MemoryBufferRef(llvm::StringRef(BC.data(), BC.size()), "<virtine-module>"),
                             mod.getContext())
          .get());
}



VirtineCompiler::VirtineCompiler(llvm::Function &source) {
  m_mod = copy_module(*source.getParent());
  // the target function
  m_func = mod().getFunction(source.getName());
  m_func->setSection("");
  // construct the argument structure
  m_arg_struct = VirtineCompiler::create_argument_struct(func());

  // The virtine runtime system requires a function, `__virtine_main` that is
  // called from the "bootloader". This call creates that function and then we
  // remove all functions and globals that arent used.
  auto *vmain = create_main(source);
  while (1) {
    if (single_dead_code_pass(*vmain) == false) break;
  }

  // TODO: analysis pass!
}


bool VirtineCompiler::single_dead_code_pass(llvm::Function &root) {
  bool did_remove = false;
  std::set<llvm::Function *> fndel;
  std::set<llvm::GlobalVariable *> gvdel;
  // Erase dead function prototypes.
  for (auto &F : mod()) {
    if (&F == &root) continue;
    // Function must be a prototype and unused.
    if (F.use_empty()) {
      fndel.insert(&F);
    }
  }

  for (auto &val : fndel) {
    // llvm::errs() << "Removing Function " << val->getName() << "\n";
    did_remove = true;

    val->eraseFromParent();
  }
  // Erase dead global var prototypes.
  for (auto I = mod().global_begin(), E = mod().global_end(); I != E;) {
    llvm::GlobalVariable *GV = &*I++;
    // Global must be a prototype and unused.
    if (GV->use_empty()) {
      gvdel.insert(GV);
    }
  }

  for (auto &val : gvdel) {
    did_remove = true;
    // llvm::errs() << "Removing Global " << val->getName() << "\n";
    val->eraseFromParent();
  }

  return did_remove;
}

/* STATIC */ llvm::StructType *VirtineCompiler::create_argument_struct(llvm::Function &fn, bool surrogate) {
  std::vector<llvm::Type *> arg_types;
  // the first set of fields in the struct are the arguments to the
  // virtine
  for (auto &arg : fn.args()) {
    auto *t = arg.getType();

    if (surrogate && t->isPointerTy()) {
      auto *ptr = (llvm::PointerType *)t;
      t = ptr->getElementType();
    }
    arg_types.push_back(t);
  }
  // the last field is the return value
  arg_types.push_back(fn.getReturnType());

  return llvm::StructType::create(arg_types);  //, surrogate ? "virtine_args_surrogate" : "virtine_args");
}




llvm::Function *VirtineCompiler::create_main(llvm::Function &source) {
  auto argtype = create_argument_struct(func(), true);
  auto argtype_ptr = llvm::PointerType::get(argtype, 0);

  auto &ctx = func().getContext();
  auto *mod = func().getParent();
  auto vmain_type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), std::vector<llvm::Type *>{argtype_ptr}, false);

  auto *vmain = llvm::Function::Create(vmain_type, llvm::Function::ExternalLinkage, "__virtine_main", mod);


  auto bb = llvm::BasicBlock::Create(ctx, "entry", vmain);
  llvm::IRBuilder<> builder(ctx);
  builder.SetInsertPoint(bb);

  bool require_surrogate_pointers = false;

  for (auto &arg : func().args()) {
    if (arg.getType()->isPointerTy()) {
      require_surrogate_pointers = true;
    }
  }


  auto virtine_args =
      vmain->getArg(0);  // builder.CreatePtrToInt(llvm::ConstantInt::get(ctx, llvm::APInt(32, 0, true)), argtype_ptr);

  // builder.CreateStore(llvm::ConstantPointerNull::get(argptr),
  // vmain->getArg(0));

  // these are the arguments to the virtine
  std::vector<llvm::Value *> argv;
  int i = 0;
  // for each argument in the virtine, read the value out of
  for (auto &arg : func().args()) {
    if (arg.getType()->isPointerTy()) {
      std::vector<llvm::Value *> indices;
      indices.push_back(llvm::ConstantInt::get(ctx, llvm::APInt(32, 0, true)));
      indices.push_back(llvm::ConstantInt::get(ctx, llvm::APInt(32, i++, true)));
      auto ptr = builder.CreateGEP(argtype, virtine_args, indices);

      argv.push_back(ptr);
    } else {
      std::vector<llvm::Value *> indices;
      indices.push_back(llvm::ConstantInt::get(ctx, llvm::APInt(32, 0, true)));
      indices.push_back(llvm::ConstantInt::get(ctx, llvm::APInt(32, i++, true)));

      auto ptr = builder.CreateGEP(argtype, virtine_args, indices);
      argv.push_back(builder.CreateLoad(ptr));
    }
  }


  auto return_value = builder.CreateCall(&func(), argv);


  /*
   * Now that we have run the virtine by calling wasp, we need to read the
   * return value from the argument structure
   */
  {
    std::vector<llvm::Value *> indices(2);
    indices[0] = llvm::ConstantInt::get(ctx, llvm::APInt(32, 0, true));
    indices[1] = llvm::ConstantInt::get(ctx, llvm::APInt(32, i, true));
    auto ptr = builder.CreateGEP(argtype, virtine_args, indices);
    builder.CreateStore(return_value, ptr);
  }


  builder.CreateRet(nullptr);

	/*
  printf("===================================================================\n");
  vmain->print(llvm::errs(), NULL);
  printf("===================================================================\n");
	*/

  return vmain;
}


// this is bad, I feel bad.
int file_exists(char *fname) {
  FILE *file;
  if ((file = fopen(fname, "r"))) {
    fclose(file);
    return 1;
  }
  return 0;
}



// the fact that this is needed in C++ is crazy...
// There really needs to be a `string::format(fmt, ...)`
#define FMT(n, ...)                \
  ({                               \
    char buf[n];                   \
    snprintf(buf, n, __VA_ARGS__); \
    std::string b(buf);            \
    b;                             \
  })

std::vector<uint8_t> VirtineCompiler::compile(void) {
  const char *fname = func().getName().data();
  auto ir_path = FMT(50, "/tmp/virtine_%s.ll", fname);
  auto obj_path = FMT(50, "/tmp/virtine_%s.o", fname);
  auto rto_path = FMT(50, "/tmp/virtine_%s.rt.o", fname);
  auto obj_final = FMT(50, "/tmp/virtine_%s.final.o", fname);
  auto bin_final = FMT(50, "/tmp/virtine_%s.final.bin", fname);


  auto runtime_src = FMT(50, "runtime_%s.s", fname);
  auto runtime_obj = FMT(50, "/tmp/runtime_%s.s.o", fname);

  // open the destination file for the IR
  int fd = open(ir_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  llvm::raw_fd_ostream out(fd, false, true);
  // write the module to the file
  mod().print(out, nullptr);
  out.flush();

  // compile the llvm-IR into assembly using LLC instead of in this
  // process (Creating a second AOT compiler is too much work)
  ck::exec("llc", "-march=x86-64", "-filetype=obj", "-O3", ir_path, "-o", obj_path);

  // compile the bootloader
  ck::exec("nasm", "-f", "elf64", "/usr/local/lib/virtine/boot64.asm", "-o", rto_path);

  std::vector<std::string> final_objects;
  final_objects.push_back(rto_path);
  final_objects.push_back(obj_path);
  final_objects.push_back("/usr/local/lib/virtine/virtine_libc.a");
  final_objects.push_back("/usr/local/lib/virtine/virtine_libm.a");

	std::cout << "runtime src: " << runtime_src << std::endl;
  // check if a runtime assembly file exists?
  if (access(runtime_src.data(), F_OK) != -1) {
	std::cout << "found runtime src: " << runtime_src << std::endl;
    ck::exec("gcc", "-c", "-no-pie", "-o", runtime_obj, runtime_src);
    final_objects.push_back(runtime_obj);
  }
  // ld -T $linker -o $tmp/out.elf $tmp/boot.o $tmp/source.o
  // ck::exec("ld", "-T", "/usr/local/lib/virtine/link64.ld", "-o", obj_final, rto_path, obj_path,
  // "/usr/local/lib/virtine/virtine_libc.a");
  ck::command link_cmd("ld", "-T", "/usr/local/lib/virtine/link64.ld", "-o", obj_final);
  for (auto &obj : final_objects) link_cmd.arg(obj);
  link_cmd.exec();

  // convert the binary image to a raw binary
  ck::exec("objcopy", "-O", "binary", obj_final, bin_final);


  FILE *fp = fopen(bin_final.c_str(), "r");




  fseek(fp, 0L, SEEK_END);
  auto code = std::vector<uint8_t>(ftell(fp));
  fseek(fp, 0L, SEEK_SET);
  fread((void *)code.data(), code.size(), 1, fp);

  fclose(fp);

  // just use rm to delete all the old stuff cause I am *very* lazy
  // ck::exec("rm", obj_path, ir_path, rto_path, obj_final, bin_final);

  return code;
}


class TypeFinder {
  std::set<const llvm::Value *> VisitedConstants;
  std::set<const llvm::MDNode *> VisitedMetadata;
  std::set<llvm::Type *> VisitedTypes;

  std::vector<llvm::StructType *> StructTypes;


 public:
  using iterator = std::set<llvm::Type *>::iterator;
  using const_iterator = std::set<llvm::Type *>::const_iterator;

  iterator begin() { return VisitedTypes.begin(); }
  iterator end() { return VisitedTypes.end(); }

  const_iterator begin() const { return VisitedTypes.begin(); }
  const_iterator end() const { return VisitedTypes.end(); }


  void incorporateType(llvm::Type *Ty) {
    // Check to see if we've already visited this type.
    if (!VisitedTypes.insert(Ty).second) return;

    llvm::SmallVector<llvm::Type *, 4> TypeWorklist;
    TypeWorklist.push_back(Ty);
    do {
      Ty = TypeWorklist.pop_back_val();

      // If this is a structure or opaque type, add a name for the type.
      if (llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(Ty))
        if (STy->hasName()) StructTypes.push_back(STy);

      // Add all unvisited subtypes to worklist for processing
      for (llvm::Type::subtype_reverse_iterator I = Ty->subtype_rbegin(), E = Ty->subtype_rend(); I != E; ++I)
        if (VisitedTypes.insert(*I).second) TypeWorklist.push_back(*I);
    } while (!TypeWorklist.empty());
  }



  void incorporateMDNode(const llvm::MDNode *V) {
    // Already visited?
    if (!VisitedMetadata.insert(V).second) return;

    // Look in operands for types.
    for (llvm::Metadata *Op : V->operands()) {
      if (!Op) continue;
      if (auto *N = llvm::dyn_cast<llvm::MDNode>(Op)) {
        incorporateMDNode(N);
        continue;
      }
      if (auto *C = llvm::dyn_cast<llvm::ConstantAsMetadata>(Op)) {
        incorporate_value(C->getValue());
        continue;
      }
    }
  }
  void incorporate_value(const llvm::Value *V) {
    if (const auto *M = llvm::dyn_cast<llvm::MetadataAsValue>(V)) {
      if (const auto *N = llvm::dyn_cast<llvm::MDNode>(M->getMetadata())) return incorporateMDNode(N);
      if (const auto *MDV = llvm::dyn_cast<llvm::ValueAsMetadata>(M->getMetadata()))
        return incorporate_value(MDV->getValue());
      return;
    }

    if (!llvm::isa<llvm::Constant>(V) || llvm::isa<llvm::GlobalValue>(V)) return;

    // Already visited?
    if (!VisitedConstants.insert(V).second) return;

    // Check this type.
    incorporateType(V->getType());

    // If this is an instruction, we incorporate it separately.
    if (llvm::isa<llvm::Instruction>(V)) return;

    // Look in operands for types.
    const auto *U = llvm::cast<llvm::User>(V);
    for (const auto &I : U->operands()) incorporate_value(&*I);
  }


  void on_module(llvm::Module &mod) {
    // Get types from global variables.
    for (const auto &G : mod.globals()) {
      incorporateType(G.getType());
      if (G.hasInitializer()) incorporate_value(G.getInitializer());
    }

    // Get types from aliases.
    for (const auto &A : mod.aliases()) {
      incorporateType(A.getType());
      if (const llvm::Value *Aliasee = A.getAliasee()) incorporate_value(Aliasee);
    }

    // Get types from functions.
    llvm::SmallVector<std::pair<unsigned, llvm::MDNode *>, 4> MDForInst;
    for (const auto &FI : mod) {
      incorporateType(FI.getType());

      for (const llvm::Use &U : FI.operands()) incorporate_value(U.get());

      // First incorporate the arguments.
      for (const auto &A : FI.args()) incorporate_value(&A);

      for (const llvm::BasicBlock &BB : FI)
        for (const llvm::Instruction &I : BB) {
          // Incorporate the type of the instruction.
          incorporateType(I.getType());

          // Incorporate non-instruction operand types. (We are incorporating
          // all instructions with this loop.)
          for (const auto &O : I.operands())
            if (&*O && !llvm::isa<llvm::Instruction>(&*O)) incorporate_value(&*O);

          // Incorporate types hiding in metadata.
          I.getAllMetadataOtherThanDebugLoc(MDForInst);
          // for (const auto &MD : MDForInst) incorporateMDNode(MD.second);
          // MDForInst.clear();
        }
    }
  }
};


VirtineCompiler::Analysis VirtineCompiler::run_analysis(void) {
  Analysis a;
  a.largest_int = -1;
  a.largest_float = -1;

  TypeFinder stypes;

  stypes.on_module(mod());

  for (auto *type : stypes) {
    if (auto *ity = llvm::dyn_cast<llvm::IntegerType>(type)) {
      a.largest_int = std::max((int)ity->getBitWidth(), a.largest_int);
    } else if (type->isFloatTy()) {
      // printf("float not supported yet\n");
    }
  }

  // printf("max int: %d bits\n", a.largest_int);


  return a;
}
