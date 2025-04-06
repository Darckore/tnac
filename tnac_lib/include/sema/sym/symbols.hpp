//
// Includes all headers defining different symbol types
//

#pragma once
#include "sema/sym/sym_base.hpp"
#include "sema/sym/sym_var.hpp"
#include "sema/sym/sym_func.hpp"
#include "sema/sym/sym_module.hpp"

TYPE_TO_ID_ASSOCIATION(tnac::semantics::variable,   tnac::semantics::sym_kind::Variable);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::parameter,  tnac::semantics::sym_kind::Parameter);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::function,   tnac::semantics::sym_kind::Function);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::module_sym, tnac::semantics::sym_kind::Module);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::scope_ref,  tnac::semantics::sym_kind::ScopeRef);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::deferred,   tnac::semantics::sym_kind::Deferred);