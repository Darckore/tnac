//
// Includes all headers defining different AST node types
//

#pragma once

#include "ast/ast.hpp"
#include "ast/ast_expr.hpp"
#include "ast/ast_util.hpp"
#include "ast/ast_decls.hpp"

TYPE_TO_ID_ASSOCIATION(tnac::ast::error_expr, tnac::ast::node_kind::Error);

TYPE_TO_ID_ASSOCIATION(tnac::ast::scope, tnac::ast::node_kind::Scope);

TYPE_TO_ID_ASSOCIATION(tnac::ast::lit_expr,    tnac::ast::node_kind::Literal);
TYPE_TO_ID_ASSOCIATION(tnac::ast::id_expr,     tnac::ast::node_kind::Identifier);
TYPE_TO_ID_ASSOCIATION(tnac::ast::unary_expr,  tnac::ast::node_kind::Unary);
TYPE_TO_ID_ASSOCIATION(tnac::ast::binary_expr, tnac::ast::node_kind::Binary);
TYPE_TO_ID_ASSOCIATION(tnac::ast::assign_expr, tnac::ast::node_kind::Assign);
TYPE_TO_ID_ASSOCIATION(tnac::ast::paren_expr,  tnac::ast::node_kind::Paren);
TYPE_TO_ID_ASSOCIATION(tnac::ast::typed_expr,  tnac::ast::node_kind::Typed);
TYPE_TO_ID_ASSOCIATION(tnac::ast::call_expr,   tnac::ast::node_kind::Call);
TYPE_TO_ID_ASSOCIATION(tnac::ast::result_expr, tnac::ast::node_kind::Result);
TYPE_TO_ID_ASSOCIATION(tnac::ast::ret_expr,    tnac::ast::node_kind::Ret);
TYPE_TO_ID_ASSOCIATION(tnac::ast::decl_expr,   tnac::ast::node_kind::Decl);
TYPE_TO_ID_ASSOCIATION(tnac::ast::matcher,     tnac::ast::node_kind::Matcher);
TYPE_TO_ID_ASSOCIATION(tnac::ast::pattern,     tnac::ast::node_kind::Pattern);
TYPE_TO_ID_ASSOCIATION(tnac::ast::cond_short,  tnac::ast::node_kind::CondShort);
TYPE_TO_ID_ASSOCIATION(tnac::ast::cond_expr,   tnac::ast::node_kind::Cond);

TYPE_TO_ID_ASSOCIATION(tnac::ast::var_decl,   tnac::ast::node_kind::VarDecl);
TYPE_TO_ID_ASSOCIATION(tnac::ast::func_decl,  tnac::ast::node_kind::FuncDecl);
TYPE_TO_ID_ASSOCIATION(tnac::ast::param_decl, tnac::ast::node_kind::ParamDecl);