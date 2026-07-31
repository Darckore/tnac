#include "output/common.hpp"
#include "output/formatting.hpp"
#include "eval/value/value.hpp"
#include "eval/value/type_impl.hpp"
#include "sema/sym/symbols.hpp"
#include "eval/console.hpp"

namespace tnac::rt::out
{
  // Special members

  value_printer::~value_printer() noexcept = default;

  value_printer::value_printer() noexcept = default;

  // Public members

  void value_printer::operator()(const eval::value& val, int base, out_stream& os) noexcept
  {
    m_out = &os;

    VALUE_GUARD(m_base, base);
    print_value(val);
  }

  void value_printer::operator()(const eval::value& val, int base) noexcept
  {
    this->operator()(val, base, out());
  }

  // Private members

  out_stream& value_printer::out() noexcept
  {
    return *m_out;
  }

  void value_printer::print_value(const eval::value& val) noexcept
  {
    eval::on_value(val, [this](auto val)
      {
        using vt = decltype(val);
        using eval::int_type;
        using eval::float_type;
        using eval::bool_type;
        using eval::array_type;
        if constexpr (utils::same_noquals<vt, int_type>)
        {
          eval::console::write_int(out(), val, m_base);
        }
        else if constexpr (utils::same_noquals<vt, float_type>)
        {
          eval::console::write_float(out(), val);
        }
        else if constexpr (utils::same_noquals<vt, bool_type>)
        {
          eval::console::write_bool(out(), val, true);
        }
        else if constexpr (utils::same_noquals<vt, array_type>)
        {
          out() << "[ ";
          const auto end = val->end();
          for (auto it = val->begin(); it != val->end(); ++it)
          {
            print_value(*it);
            if (std::next(it) != end)
              out() << ", ";
          }
          out() << " ]";
        }
        else
        {
          out() << val;
        }
      });
  }
}