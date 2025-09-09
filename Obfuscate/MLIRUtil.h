// #pragma once
// #include "Util.h"
// #include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
// #include "mlir/Dialect/LLVMIR/LLVMDialect.h"
// #include "mlir/Dialect/SCF/IR/SCF.h"
// #include "mlir/IR/Dialect.h"
// #include "mlir/IR/MLIRContext.h"
// #include "mlir/InitAllDialects.h"
// #include "mlir/InitAllPasses.h"
// #include "mlir/Pass/Pass.h"
// #include "mlir/Pass/PassManager.h"
// #include "mlir/Target/LLVMIR/Export.h"
// #include "mlir/Target/LLVMIR/Import.h"
// #include <llvm-17/llvm/IR/Module.h>
// #include <llvm-17/llvm/Transforms/Utils/Cloning.h>
// #include <memory>
// #include <mlir/IR/Builders.h>
// #include <mlir/IR/Operation.h>
// #include <mlir/IR/PatternMatch.h>
// #include <mlir/Support/LogicalResult.h>
// #include <optional>
// inline mlir::DialectRegistry newRegistry() {
//   mlir::DialectRegistry registry;
//   registry.insert<mlir::func::FuncDialect>();
//   registry.insert<mlir::arith::ArithDialect>();
//   registry.insert<mlir::cf::ControlFlowDialect>();
//   registry.insert<mlir::scf::SCFDialect>();
//   registry.insert<mlir::LLVM::LLVMDialect>();
//   return registry;
// }
// template <typename F> auto mlirPass(F f, llvm::Module &m) {
//   auto context = std::make_shared<mlir::MLIRContext>();
//   auto n = mlir::translateLLVMIRToModule(llvm::CloneModule(m), &*context);
//   auto r = f(&n, context);
//   auto o = mlir::translateModuleToLLVMIR(n.get(), m.getContext());
//   clearModule(&m);
//   llvm::Linker::linkModules(m, std::move(o));
//   return r;
// };
// template <typename T> struct ObfuscationCreator {
//   virtual std::optional<mlir::Operation *>
//   create(T input, mlir::OpBuilder rewriter) const = 0;
// };
// using ObfuscationRewrite = ObfuscationCreator<mlir::Operation *>;
// using ObfuscationSummoner = ObfuscationCreator<mlir::Type>;
// template <typename T> struct Chain : ObfuscationCreator<T> {
//   std::shared_ptr<ObfuscationCreator<T>> first;
//   std::shared_ptr<ObfuscationRewrite> second;
//   Chain(std::shared_ptr<ObfuscationCreator<T>> first,
//         std::shared_ptr<ObfuscationRewrite> second)
//       : first(first), second(second) {}
//   std::optional<mlir::Operation *>
//   create(T input, mlir::OpBuilder rewriter) const override {
//     std::optional<mlir::Operation *> a = first->create(input, rewriter);
//     if (!a.has_value())
//       return a;
//     return second->create(a.value(), rewriter);
//   }
// };
// std::shared_ptr<ObfuscationRewrite>
// createIf(std::shared_ptr<ObfuscationRewrite> wrapped,
//          std::shared_ptr<ObfuscationSummoner> cond);
// struct Pattern : mlir::RewritePattern {
//   std::shared_ptr<ObfuscationRewrite> rewrite;
//   inline Pattern(mlir::MLIRContext *ctx,
//                  std::shared_ptr<ObfuscationRewrite> rewrite)
//       : mlir::RewritePattern(MatchAnyOpTypeTag{}, 1, ctx), rewrite(rewrite) {}
//       mlir::LogicalResult matchAndRewrite(mlir::Operation *op, mlir::PatternRewriter &rewriter) const override;
// };
