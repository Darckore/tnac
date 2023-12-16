#include "compiler/detail/module_info.hpp"
#include "cfg/ir/ir.hpp"

namespace tnac::detail
{
  // Special members

  module_info::~module_info() noexcept = default;

  module_info::module_info() noexcept = default;


  // Public members

  void module_info::store(module_sym& sym, module_def& def) noexcept
  {
    auto newIt = m_data.try_emplace(&sym, &def);
    UTILS_ASSERT(newIt.second || newIt.first->second == &def);
  }

  module_info::module_def* module_info::locate(module_sym& sym) noexcept
  {
    auto found = m_data.find(&sym);
    return found != m_data.end() ? found->second : nullptr;
  }

  void module_info::wipe() noexcept
  {
    m_data.clear();
    m_stack.clear();
  }

  void module_info::push(module_sym& sym) noexcept
  {
    m_stack.push_back(&sym);
  }

  module_info::module_sym* module_info::pop() noexcept
  {
    if (m_stack.empty())
      return {};

    auto sym = m_stack.back();
    m_stack.pop_back();
    return sym;
  }

  void module_info::enter_module(ir::function& mod) noexcept
  {
    exit_module();
    m_curModule = &mod;
    enter_function(mod);
  }
  void module_info::exit_module() noexcept
  {
    exit_function();
    m_curModule = {};
  }
  ir::function* module_info::try_current_module() noexcept
  {
    return m_curModule;
  }
  bool module_info::has_current_module() const noexcept
  {
    return static_cast<bool>(m_curModule);
  }
  ir::function& module_info::current_module() noexcept
  {
    UTILS_ASSERT(has_current_module());
    return *try_current_module();
  }

  void module_info::enter_function(ir::function& fn) noexcept
  {
    exit_function();
    m_curFunction = &fn;
  }
  void module_info::exit_function() noexcept
  {
    if (!m_curFunction)
      return;

    m_curFunction = m_curFunction->owner_func();
  }
  ir::function* module_info::try_current_function() noexcept
  {
    return m_curFunction;
  }
  bool module_info::has_current_function() const noexcept
  {
    return static_cast<bool>(m_curFunction);
  }
  ir::function& module_info::current_function() noexcept
  {
    UTILS_ASSERT(has_current_function());
    return *try_current_function();
  }
}