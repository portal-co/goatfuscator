// #include "MLIRUtil.h"
// #include <memory>
// #include <mlir/Dialect/SCF/IR/SCF.h>
// #include <mlir/IR/Builders.h>
// #include <mlir/IR/BuiltinTypes.h>
// #include <mlir/IR/OpDefinition.h>
// #include <mlir/IR/PatternMatch.h>
// #include <mlir/Support/LogicalResult.h>
// namespace {
// struct If : ObfuscationRewrite {
//   std::shared_ptr<ObfuscationRewrite> wrapped;
//   std::shared_ptr<ObfuscationSummoner> cond;
//   std::optional<mlir::Operation *>
//   create(mlir::Operation *input, mlir::OpBuilder rewriter) const override {
//     if (input->hasTrait<mlir::OpTrait::IsTerminator>())
//       return {input};
//     auto c = cond->create(rewriter.getI1Type(), rewriter);
//     if (!c.has_value()) {
//       return {};
//     }
//     auto n = rewriter.create<mlir::scf::IfOp>(input->getLoc(),
//                                               input->getResultTypes(),
//                                               c.value()->getResult(0), true);
//     auto tb = n.getThenBodyBuilder(rewriter.getListener());
//     auto then = wrapped->create(input->clone(), tb);
//     if (!then.has_value()) {
//       return {};
//     }
//     tb.create<mlir::scf::YieldOp>(input->getLoc(), then.value()->getResults());
//     auto eb = n.getElseBodyBuilder(rewriter.getListener());
//     auto els = wrapped->create(input->clone(), eb);
//     if (!els.has_value()) {
//       return {};
//     }
//     eb.create<mlir::scf::YieldOp>(input->getLoc(), els.value()->getResults());
//     return n;
//   }
// };
// } // namespace
// std::shared_ptr<ObfuscationRewrite>
// createIf(std::shared_ptr<ObfuscationRewrite> wrapped,
//          std::shared_ptr<ObfuscationSummoner> cond) {
//   return std::make_shared<If>(wrapped, cond);
// }
// mlir::LogicalResult
// Pattern::matchAndRewrite(mlir::Operation *op,
//                          mlir::PatternRewriter &rewriter) const {
//   auto x = rewrite->create(op, rewriter);
//   if (!x.has_value())
//     return mlir::failure();
//   rewriter.replaceOp(op, x.value());
//   return mlir::success();
// }