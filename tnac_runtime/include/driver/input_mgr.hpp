//
// Input manager
//

#pragma once
#include "ast/ast_nodes.hpp"
#include "src_mgr/source_manager.hpp"

namespace tnac_rt
{
  namespace detail
  {
    using input_value = std::variant<tnac::buf_t, tnac::src::file*>;

    template <typename T>
    concept source = requires(T t)
    {
      input_value{ std::move(t) };
    };
  }

  //
  // The input data stored inside the input manager's collection
  //
  class input final
  {
  public:
    using buf_t  = tnac::buf_t;
    using str_t  = tnac::string_t;
    using file_t = tnac::src::file;

    using value_type = std::variant<buf_t, file_t*>;

  public:
    CLASS_SPECIALS_NONE(input);

    input(buf_t buf) noexcept :
      m_stored{ std::move(buf) }
    {}

    input(file_t* file) noexcept :
      m_stored{ file }
    {
      UTILS_ASSERT(static_cast<bool>(file));
    }

  public:
    str_t buffer() const noexcept
    {
      constexpr utils::visitor vis
      {
        [](const buf_t& buf) noexcept
        {
          return str_t{ buf };
        },
        [](file_t* file) noexcept
        {
          return *file->get_contents();
        }
      };
      return std::visit(vis, m_stored);
    }

    file_t* try_get_file() noexcept
    {
      auto fptr = std::get_if<file_t*>(&m_stored);
      if (!fptr)
        return {};

      return *fptr;
    }

  private:
    value_type m_stored{};
  };

  //
  // Manages the input buffer and reports errors
  //
  class input_mgr final
  {
  public:
    using stored_input  = tnac_rt::input;
    using input_storage = std::unordered_map<std::uint32_t, input>;
    using src_mgr       = tnac::source_manager;
    using loc_t         = tnac::src::location;
    using loc_wrapper   = tnac::src::loc_wrapper;

  public:
    CLASS_SPECIALS_NONE(input_mgr);

    ~input_mgr() noexcept;

    explicit input_mgr(src_mgr& mgr) noexcept;

  public:
    //
    // Stores the next input string
    //
    stored_input& input(tnac::buf_t in) noexcept;

    //
    // Reads input from a file and stores it
    //
    stored_input* from_file(tnac::string_t fname) noexcept;

    //
    // Prints an error message by token pos
    //
    void on_error(const tnac::token& tok, tnac::string_t msg) noexcept;

    //
    // Handler for parse errors
    //
    void on_parse_error(const tnac::ast::error_expr& err) noexcept;

  private:
    //
    // Prints location info
    //
    void print_location(loc_wrapper at) noexcept;

    //
    // Returns a reference to the err stream
    //
    out_stream& err() noexcept;

    //
    // Stores the input and returns a reference to it
    //
    template <detail::source Src>
    stored_input& store(input_storage& storage, Src&& src) noexcept
    {
      auto&& idx = &storage == &m_input ? m_inputIdx : m_fileIdx;
      auto newItem = storage.try_emplace(idx++, std::forward<Src>(src));
      return newItem.first->second;
    }

  private:
    input_storage m_input;
    input_storage m_files;
    src_mgr& m_srcMgr;
    std::uint32_t m_inputIdx{};
    std::uint32_t m_fileIdx{};

    out_stream* m_err{ &std::cerr };
  };
}