//
// Includes all headers defining different IR node types
//

#pragma once

#include "cfg/ir/ir_base.hpp"
#include "cfg/ir/ir_function.hpp"
#include "cfg/ir/ir_instructions.hpp"
#include "cfg/ir/ir_basic_block.hpp"

TYPE_TO_ID_ASSOCIATION(tnac::ir::function,    tnac::ir::ir_kind::Function);
TYPE_TO_ID_ASSOCIATION(tnac::ir::basic_block, tnac::ir::ir_kind::Block);
TYPE_TO_ID_ASSOCIATION(tnac::ir::instruction, tnac::ir::ir_kind::Instruction);