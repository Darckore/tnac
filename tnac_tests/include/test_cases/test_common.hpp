#pragma once
#include "parser/parser.hpp"
#include "sema/sema.hpp"

namespace tnac_tests
{
  namespace detail
  {
    namespace tree = tnac::ast;
    using tree::node_kind;
    using tnac::string_t;

    struct parse_helper
    {
      parse_helper() :
        parser{ builder, sema }
      {}

      auto operator()(string_t input) noexcept
      {
        return parser(input);
      }

      tnac::ast::builder builder;
      tnac::sema sema;
      tnac::parser parser;
    };
  }
}