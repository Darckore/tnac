#include "compiler/detail/context.hpp"
#include "cfg/ir/ir.hpp"

namespace tnac::detail
{
  // Special members

  context::~context() noexcept = default;

  context::context() noexcept = default;


  // Public members

  void context::store(module_sym& sym, module_def& def) noexcept
  {
    auto newIt = m_data.try_emplace(&sym, &def);
    UTILS_ASSERT(newIt.second || newIt.first->second == &def);
  }

  context::module_def* context::locate(module_sym& sym) noexcept
  {
    auto found = m_data.find(&sym);
    return found != m_data.end() ? found->second : nullptr;
  }

  void context::wipe() noexcept
  {
    drop_func();

    m_curFunction = {};
    m_curModule = {};

    m_data.clear();
  }

  void context::push(module_sym& sym) noexcept
  {
    m_stack.push_back(&sym);
  }

  context::module_sym* context::pop() noexcept
  {
    if (m_stack.empty())
      return {};

    auto sym = m_stack.back();
    m_stack.pop_back();
    return sym;
  }

  void context::enter_module(ir::function& mod) noexcept
  {
    exit_module();
    m_curModule = &mod;
    enter_function(mod);
  }
  void context::exit_module() noexcept
  {
    exit_function();
    m_curModule = {};
  }
  ir::function& context::current_module() noexcept
  {
    UTILS_ASSERT(m_curModule);
    return *m_curModule;
  }

  void context::enter_function(ir::function& fn) noexcept
  {
    exit_function();
    m_curFunction = &fn;
  }
  void context::exit_function() noexcept
  {
    if (!m_curFunction)
      return;

    drop_func();
    m_curFunction = m_curFunction->owner_func();
  }
  ir::function& context::current_function() noexcept
  {
    UTILS_ASSERT(m_curFunction);
    return *m_curFunction;
  }

  ir::basic_block& context::create_block(string_t name) noexcept
  {
    return current_function().create_block(name);
  }

  void context::enqueue_block(ir::basic_block& block) noexcept
  {
    m_blocks.push(&block);
  }

  ir::basic_block& context::current_block() noexcept
  {
    UTILS_ASSERT(!m_blocks.empty());
    return *m_blocks.front();
  }

  ir::basic_block* context::terminal_block() noexcept
  {
    return m_terminal;
  }

  ir::basic_block& context::terminal_or_entry() noexcept
  {
    return m_terminal ? *m_terminal : current_function().entry();
  }

  void context::terminate_at(ir::basic_block& term) noexcept
  {
    m_terminal = &term;
  }

  void context::exit_block() noexcept
  {
    UTILS_ASSERT(!m_blocks.empty());
    m_blocks.pop();
  }

  void context::func_start_at(ir::instruction& instr) noexcept
  {
    if (!m_funcFirst)
      m_funcFirst = instr.to_iterator();
  }

  context::instr_iter context::funct_start() noexcept
  {
    return m_funcFirst;
  }


  // Private members

  void context::drop_func() noexcept
  {
    UTILS_ASSERT(m_stack.empty());
    m_stack.clear();

    UTILS_ASSERT(m_blocks.empty());
    while (!m_blocks.empty())
      m_blocks.pop();

    m_terminal = {};
    m_funcFirst = {};
  }
}