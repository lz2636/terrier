#pragma once

#include <vector>

#include "catalog/schema.h"
#include "execution/compiler/ast_fwd.h"
#include "execution/compiler/operator/operator_translator.h"

namespace terrier::planner {
class InsertPlanNode;
}  // namespace terrier::planner

namespace terrier::execution::compiler {

/**
 * InsertTranslator
 */
class InsertTranslator : public OperatorTranslator {
 public:
  /**
   * Create a new translator for the given insert plan. The compilation occurs within the
   * provided compilation context and the operator is participating in the provided pipeline.
   * @param plan The plan.
   * @param compilation_context The context of compilation this translation is occurring in.
   * @param pipeline The pipeline this operator is participating in.
   */
  InsertTranslator(const planner::InsertPlanNode &plan, CompilationContext *compilation_context, Pipeline *pipeline);

  /**
   * Does nothing.
   * @param decls The top-level declarations.
   */
  void DefineHelperFunctions(util::RegionVector<ast::FunctionDecl *> *decls) override {}

  /**
   * Does nothing.
   * @param pipeline The current pipeline.
   * @param function The pipeline generating function.
   */
  void InitializePipelineState(const Pipeline &pipeline, FunctionBuilder *function) const override {}

  /**
   * Implement insertion logic where it fills in the insert PR obtained from the StorageInterface struct
   * with values from the child.
   * @param context The context of the work.
   * @param function The pipeline generating function.
   */
  void PerformPipelineWork(WorkContext *context, FunctionBuilder *function) const override;

  /**
   * @return The child's output at the given index.
   */
  ast::Expr *GetChildOutput(WorkContext *context, uint32_t child_idx, uint32_t attr_idx) const override;

  /**
   * @return An expression representing the value of the column with the given OID.
   */
  ast::Expr *GetTableColumn(catalog::col_oid_t col_oid) const override;

 private:
  // Declare storage interface.
  void DeclareInserter(FunctionBuilder *builder) const;

  // Free the storage interface.
  void GenInserterFree(FunctionBuilder *builder) const;

  // Sets the oids that we are inserting on, using the schema from the insert plan node.
  void SetOids(FunctionBuilder *builder) const;

  // Declares the projected row that we will be using the insert values with.
  void DeclareInsertPR(FunctionBuilder *builder) const;

  // Gets the projected row pointer that we will fill in with values to insert.
  void GetInsertPR(FunctionBuilder *builder) const;

  // Sets the values in the projected row which we will use to insert into the table.
  void GenSetTablePR(FunctionBuilder *builder, WorkContext *context, uint32_t idx) const;

  // Insert into the table.
  void GenTableInsert(FunctionBuilder *builder) const;

  // Insert into an index of this table.
  void GenIndexInsert(WorkContext *context, FunctionBuilder *builder, const catalog::index_oid_t &index_oid) const;

  // Gets all the column oids in a schema.
  static std::vector<catalog::col_oid_t> AllColOids(const catalog::Schema &table_schema_) {
    std::vector<catalog::col_oid_t> oids;
    for (const auto &col : table_schema_.GetColumns()) {
      oids.emplace_back(col.Oid());
    }
    return oids;
  }

  // Storage interface inserter struct which we use to insert.
  ast::Identifier inserter_;

  // Projected row that the inserter spits out for us to insert with.
  ast::Identifier insert_pr_;

  // Column oids that we are inserting on.
  ast::Identifier col_oids_;

  // Schema of the table that we are inserting on.
  const catalog::Schema &table_schema_;

  // All the oids that we are inserting on.
  std::vector<catalog::col_oid_t> all_oids_;

  // Projection map of the table that we are inserting into.
  // This maps column oids to offsets in a projected row.
  storage::ProjectionMap table_pm_;
};

}  // namespace terrier::execution::compiler
