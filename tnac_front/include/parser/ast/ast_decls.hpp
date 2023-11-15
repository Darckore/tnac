//
// Nodes for declarations
//

#pragma once
#include "parser/ast/ast_base.hpp"
#include "parser/ast/ast_expr.hpp"

namespace tnac
{
  class sema;
}

namespace tnac::ast
{
  //
  // Base declarator for all entities that can be defined
  //
  class decl : public node
  {
  private:
    friend class builder;
    friend class sema;

  public:
    CLASS_SPECIALS_NONE(decl);

    virtual ~decl() noexcept;

  protected:
    decl(kind k, const token& id, const token& pos, node* def) noexcept;
    
    decl(kind k, const token& id, node* def) noexcept;

    //
    // Attaches a symbol to this declarator. Called sema on symbol creation
    //
    void attach_symbol(semantics::symbol& sym) noexcept;

  public:
    //
    // Returns the entity name
    //
    string_t name() const noexcept;

    //
    // Returns the definition
    // 
    // const version
    //
    const node* definition() const noexcept;

    //
    // Returns the definition
    // 
    node* definition() noexcept;

    //
    // Returns the first token associated with this declarator
    //
    const token& pos() const noexcept;

    //
    // Returns the attached symbol
    // 
    // const version
    //
    const semantics::symbol& symbol() const noexcept;

    //
    // Returns the attached symbol
    // 
    semantics::symbol& symbol() noexcept;

  private:
    node* m_def{};
    semantics::symbol* m_symbol{};
    token m_id;
    token m_pos;
  };

  inline auto get_id(const decl& d) noexcept
  {
    return d.what();
  }

  //
  // Decl expression. Holds the declarator corresponding to the declared entity
  //
  class decl_expr final : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(decl_expr);

    virtual ~decl_expr() noexcept;

  protected:
    decl_expr(decl& d) noexcept;

  public:
    //
    // Returns the declarator
    // 
    // const version
    //
    const decl& declarator() const noexcept;

    //
    // Returns the declarator
    // 
    decl& declarator() noexcept;

  private:
    decl* m_decl{};
  };


  //
  // Variable declarator
  //
  class var_decl final : public decl
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(var_decl);

    virtual ~var_decl() noexcept;

  protected:
    var_decl(const token& var, expr& initialiser) noexcept;

  public:
    //
    // Returns the initialiser which is present at the right side of the the assignment operator
    // 
    // const version
    //
    const expr& initialiser() const noexcept;

    //
    // Returns the initialiser which is present at the right side of the the assignment operator
    //
    expr& initialiser() noexcept;
  };


  //
  // Parameter declarator
  //
  class param_decl final : public decl
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(param_decl);

    virtual ~param_decl() noexcept;

  protected:
    param_decl(const token& paramName, expr* opt) noexcept;
  };


  //
  // Function declarator
  //
  class func_decl final : public decl
  {
  public:
    using param_list = std::vector<param_decl*>;
    using size_type = param_list::size_type;

  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(func_decl);

    virtual ~func_decl() noexcept;

  protected:
    func_decl(const token& func, const token& pos, scope& def, param_list params) noexcept;

  public:
    //
    // Returns function parameter list
    //
    // const version
    //
    const param_list& params() const noexcept;

    //
    // Returns function parameter list
    //
    param_list& params() noexcept;

    //
    // Returns the number of parameters
    //
    size_type param_count() const noexcept;

    //
    // Returns function body
    // 
    // const version
    //
    const scope& body() const noexcept;

    //
    // Returns function body
    // 
    scope& body() noexcept;

  private:
    param_list m_params;
  };

}