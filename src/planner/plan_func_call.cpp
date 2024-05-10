#include <memory>
#include <tuple>
#include "binder/bound_expression.h"
#include "binder/bound_statement.h"
#include "binder/expressions/bound_agg_call.h"
#include "binder/expressions/bound_alias.h"
#include "binder/expressions/bound_binary_op.h"
#include "binder/expressions/bound_column_ref.h"
#include "binder/expressions/bound_constant.h"
#include "binder/expressions/bound_func_call.h"
#include "binder/expressions/bound_unary_op.h"
#include "binder/statement/select_statement.h"
#include "common/exception.h"
#include "common/macros.h"
#include "common/util/string_util.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/expressions/column_value_expression.h"
#include "execution/expressions/constant_value_expression.h"
#include "execution/expressions/string_expression.h"
#include "execution/plans/abstract_plan.h"
#include "fmt/format.h"
#include "planner/planner.h"

namespace bustub {

// NOLINTNEXTLINE
auto Planner::GetFuncCallFromFactory(const std::string &func_name, std::vector<AbstractExpressionRef> args)
    -> AbstractExpressionRef {
  // 1. 检查解析的函数名是否为 "lower" 或 "upper"。
  // 2. 验证参数数量（应为 1），参考测试用例以确定何时抛出异常。
  // 3. 返回一个 `StringExpression` 的 std::shared_ptr。
  if (func_name == "lower" || func_name == "upper") {
    // 验证参数数量
    if (args.size() != 1) {
      throw Exception(fmt::format("Function {} requires exactly one argument", func_name));
    }

    // 将函数名转换为 StringExpressionType
    StringExpressionType expr_type = (func_name == "lower") ? StringExpressionType::Lower : StringExpressionType::Upper;
    // 创建并返回一个 StringExpression
    auto arg = args.front();
    return std::make_shared<StringExpression>(arg, expr_type);
  }

  throw Exception(fmt::format("func call {} not supported in planner yet", func_name));
}

}  // namespace bustub
