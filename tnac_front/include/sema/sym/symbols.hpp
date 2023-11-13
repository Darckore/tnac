//
// Includes all headers defining different symbol types
//

#pragma once
#include "sema/sym/sym_base.hpp"

TYPE_TO_ID_ASSOCIATION(tnac::semantics::variable,  tnac::semantics::sym_kind::Variable);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::parameter, tnac::semantics::sym_kind::Parameter);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::function,  tnac::semantics::sym_kind::Function);