/**
* @file    DerivedAttributeSymbol.h
* Declarations for the DerivedAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "MaintainedAttributeSymbol.h"
#include "NumericSymbol.h"
#include "ConstantSymbol.h"

class PartitionSymbol;
class ClassificationSymbol;
class IdentityAttributeSymbol;
class EntityInternalSymbol;

using namespace std;

/**
 * DerivedAttributeSymbol
 * 
 * Symbol for derived agentvars with various signatures
 * E.g. duration(happy, true)
 */
class DerivedAttributeSymbol : public MaintainedAttributeSymbol
{
private:
    typedef MaintainedAttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor with all arguments.
     *
     * @param agent    The agent.
     */
    explicit DerivedAttributeSymbol(const Symbol *agent,
                          token_type tok,
                          const Symbol *av1,
                          const Symbol *av2,
                          const Symbol *prt,
                          const Symbol *cls,
                          const ConstantSymbol *k1,
                          const ConstantSymbol *k2,
                          const ConstantSymbol *k3)
        : MaintainedAttributeSymbol(DerivedAttributeSymbol::member_name(tok, av1, av2, prt, cls, k1, k2, k3),
                        agent,
                        NumericSymbol::find(token::TK_double) // will be changed later
                        )
        , tok(tok)
        , av1(av1 ? av1->stable_pp() : nullptr)
        , pp_av1(nullptr)
        , av2(av2 ? av2->stable_pp() : nullptr)
        , pp_av2(nullptr)
        , prt(prt ? prt->stable_pp() : nullptr)
        , pp_prt(nullptr)
        , cls(cls ? cls->stable_pp() : nullptr)
        , pp_cls(nullptr)
        , k1(k1)
        , k2(k2)
        , k3(k3)
        , iav(nullptr)
        , dav(nullptr)
        , ait(nullptr)
        , numeric_id(0)
        , any_to_hooks(false)
    {
        validate();
        assign_sorting_group();
        create_auxiliary_symbols();
    }

    /**
     * The member name for a specific symbol of this kind.
     *
     * @param tok The token.
     * @param av1 The first av.
     * @param av2 The second av.
     * @param prt The partition.
     * @param cls The classification.
     * @param k1  The first ConstantSymbol.
     * @param k2  The second ConstantSymbol.
     * @param k3  The third ConstantSymbol.
     *
     * @return The member namne as a string.
     */
    static string member_name(token_type tok,
                              const Symbol *av1,
                              const Symbol *av2,
                              const Symbol *prt,
                              const Symbol *cls,
                              const ConstantSymbol *k1,
                              const ConstantSymbol *k2,
                              const ConstantSymbol *k3);

    /**
     * Creates the given symbol, or returns it if it already exists.
     *
     * @param agent    The agent.
     * @param tok      The token.
     * @param av1      The first av.
     * @param av2      The second av.
     * @param prt      The partition.
     * @param cls      The classification.
     * @param k1       The first ConstantSymbol.
     * @param k2       The second ConstantSymbol.
     * @param k3       The third ConstantSymbol.
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1,
                                  const Symbol *av2,
                                  const Symbol *prt,
                                  const Symbol *cls,
                                  const ConstantSymbol *k1,
                                  const ConstantSymbol *k2,
                                  const ConstantSymbol *k3);

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tok      The token.
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok)
    {
        // signature conditions:
        assert(agent);
        return create_symbol(agent, tok, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tok      The token.
     * @param av1      The agentvar.
     * @param k1       The constant.
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1,
                                  const ConstantSymbol *k1)
    {
        // signature conditions:
        assert(agent);
        assert(av1);
        assert(k1);
        return create_symbol(agent, tok, av1, nullptr, nullptr, nullptr, k1, nullptr, nullptr);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tok      The token.
     * @param av1      The agentvar.
     * @param k1       The constant.
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1,
                                  const ConstantSymbol *k1,
                                  const ConstantSymbol *k2)
    {
        // signature conditions:
        assert(agent);
        assert(av1);
        assert(k1);
        assert(k2);
        return create_symbol(agent, tok, av1, nullptr, nullptr, nullptr, k1, k2, nullptr);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tok      The token.
     * @param av1      The agentvar.
     * @param k1       The constant.
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1,
                                  const ConstantSymbol *k1,
                                  const ConstantSymbol *k2,
                                  const ConstantSymbol *k3)
    {
        // signature conditions:
        assert(agent);
        assert(av1);
        assert(k1);
        assert(k2);
        assert(k3);
        return create_symbol(agent, tok, av1, nullptr, nullptr, nullptr, k1, k2, k3);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tok      The token.
     * @param av1      The agentvar #1
     * @param k1       The constant #1
     * @param k2       The constant #2
     * @param av2      The agentvar #2
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1,
                                  const ConstantSymbol *k1,
                                  const ConstantSymbol *k2,
                                  const Symbol *av2)
    {
        // signature conditions:
        assert(agent);
        assert(av1);
        assert(k1);
        assert(k2);
        assert(av2);
        return create_symbol(agent, tok, av1, av2, nullptr, nullptr, k1, k2, nullptr);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tok      The token.
     * @param av1      The agentvar #1
     * @param k1       The constant.
     * @param av2      The agentvar #2
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1,
                                  const ConstantSymbol *k1,
                                  const Symbol *av2)
    {
        // signature conditions:
        if (tok == token::TK_weighted_duration) {
            // special exception to store weight in av2 if av1 is nullptr
            // (for subsequent code coherence for the two cases for weighted_duration)
            assert(agent);
            assert(av1 || !av1); // observed (optional)
            assert(av1 ? (k1 != nullptr) : true); // constant (required for observed)
            assert(av2);
        }
        else {
            assert(agent);
            assert(av1);
            assert(k1);
            assert(av2);
        }
        return create_symbol(agent, tok, av1, av2, nullptr, nullptr, k1, nullptr, nullptr);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param av1      The agentvar.
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1)
    {
        // signature conditions:
        assert(agent);
        assert(av1);
        return create_symbol(agent, tok, av1, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param av1      The agentvar (#1)
     * @param av2      The agentvar (#2)
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1,
                                  const Symbol *av2)
    {
        // signature conditions:
        assert(agent);
        assert(av1);
        assert(av2);
        return create_symbol(agent, tok, av1, av2, nullptr, nullptr, nullptr, nullptr, nullptr);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param av1      The agentvar
     * @param av2      always nullptr
     * @param prt      The partition
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1,
                                  const Symbol *av2,
                                  const Symbol *prt)
    {
        // signature conditions:
        assert(agent);
        assert(av1);
        assert(!av2); // placeholder only
        assert(prt);
        return create_symbol(agent, tok, av1, av2, prt, nullptr, nullptr, nullptr, nullptr);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param av1      The agentvar.
     * @param av2      always nullptr
     * @param prt      always nullptr
     * @param cls      The classification.
     *
     * @return The symbol.
     */
    static DerivedAttributeSymbol * create_symbol(const Symbol* agent,
                                  token_type tok,
                                  const Symbol *av1,
                                  const Symbol *av2,
                                  const Symbol *prt,
                                  const Symbol *cls)
    {
        // signature conditions:
        assert(agent);
        assert(av1);
        assert(!av2); // placeholder only
        assert(!prt); // placeholder only
        assert(cls);
        return create_symbol(agent, tok, av1, av2, prt, cls, nullptr, nullptr, nullptr);
    }

    /**
     * Validate argument signatures.
     */
    void validate();

    /**
     * Assign sorting group.
     */
    void assign_sorting_group();

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    /**
     * Perform addition syntax checks on arguments of self_scheduling attributes
     *
     * On failure, an exception is thrown
     */
    void check_ss_arguments();

    /**
     * Assign the datatype of this symbol.
     */
    void assign_data_type();

    /**
     * Assign post-parse members of this symbol.
     */
    void assign_pp_members();

    /**
     * Record symbols on which the derived attributes depends
     */
    void record_dependencies();

    /**
     * Create side-effects to maintain this symbol.
     */
    void create_side_effects();

    virtual string pretty_name() const;

    /**
     * Gets the fixed label for the derived attribute.
     * 
     * The specialization is language independent.
     *
     * @param language The language.
     *
     * @return A string.
     */
    string label(const LanguageSymbol& language) const
    {
        return pretty_name();
    }

    /**
     * Query if this derived attribute is self-scheduling.
     *
     * @return true if self-scheduling, false if not.
     */
    bool is_self_scheduling() const;

    /**
     * Query if this derived attribute is a trigger.
     *
     * @return true if a trigger, false if not.
     */
    bool is_trigger() const;

    /**
     * Name for local flag variable used in implement function of self-scheduling event
     *
     * @return The name
     */
    string flag_name() const
    {
        return "flag_" + name;
    }

    string pp_modgen_name() const;

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    /**
     * The token for the derived attribute
     */
    const token_type tok;

    /**
     * agentvar #1
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** av1;

    /**
     * agentvar #1
     * 
     * Only valid after post-parse phase 1.
     */
    AttributeSymbol *pp_av1;

    /**
     * agentvar #2
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** av2;

    /**
     * agentvar #2
     * 
     * Only valid after post-parse phase 1.
     */
    AttributeSymbol *pp_av2;

    /**
     * partition
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** prt;

    /**
     * partition
     * 
     * Only valid after post-parse phase 1.
     */
    PartitionSymbol *pp_prt;

    /**
     * classification
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** cls;

    /**
     * classification
     * 
     * Only valid after post-parse phase 1.
     */
    ClassificationSymbol *pp_cls;

    /**
     * Constant #1.
     */
    const ConstantSymbol *k1;

    /**
     * Constant #2.
     */
    const ConstantSymbol *k2;

    /**
     * Constant #3.
     */
    const ConstantSymbol *k3;

    /**
     * The identity agentvar symbol
     * 
     * Used for conditions and spells
     */
    IdentityAttributeSymbol *iav;

    /**
     * The derived agentvar symbol
     * 
     * Used for required associated derived agentvars, e.g. to implement
     * value_at_first_entrance(disease_phase,DP_INFECTIOUS,age)
     * need 
     * undergone_entrance(disease_phase,DP_INFECTIOUS)
     */
    DerivedAttributeSymbol *dav;

    /**
     * The agent internal member for time
     * 
     * Used to record the internal symbol created to hold the nect time of occurrence of the self-
     * scheduling derived agentvar.
     */
    EntityInternalSymbol *ait;

    /**
     * Numeric identifier for the symbol.
     * 
     * Used to hold numeric id for self-scheduling agentvars which appear in generated code and in
     * event trace logs.
     */
    int numeric_id;

    /**
     * Indicates if there are any hooks to this derived attribute
     */
    bool any_to_hooks;
};

