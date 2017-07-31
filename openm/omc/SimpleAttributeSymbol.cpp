/**
* @file    SimpleAttributeSymbol.cpp
* Definitions for the SimpleAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "SimpleAttributeSymbol.h"
#include "EntityForeignMemberSymbol.h"
#include "EntityFuncSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"
#include "Literal.h"

using namespace std;

string SimpleAttributeSymbol::initialization_value(bool type_default) const
{
    string result;

    if (!type_default && initializer != nullptr) {
        result = initializer->value();
    }
    else {
        result = AttributeSymbol::initialization_value(type_default);
    }

    return result;
}

CodeBlock SimpleAttributeSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // add declaration code specific to simple agentvars

	// example:         AssignableAgentVar<bool, Person, &Person::alive_side_effects> alive;
	h += "AssignableAgentVar<"
		+ pp_data_type->name + ", "
		+ pp_data_type->exposed_type() + ", "
		+ agent->name + ", "
		+ "&om_name_" + name + ", "
		+ "&" + side_effects_fn->unique_name + ", "
		+ (!side_effects_fn->empty() ? "true" : "false") + ", "
		+ "&" + notify_fn->unique_name + ", "
		+ (!notify_fn->empty() ? "true" : "false")
		+ ">";
	h += name + ";";
    return h;
}

void SimpleAttributeSymbol::post_parse(int pass)
{
	// Hook into the post_parse hierarchical calling chain
	super::post_parse(pass);

	// Perform post-parse operations specific to this level in the Symbol hierarchy.
	switch (pass) {
	case eCreateMissingSymbols:
	{
		// Determine if simple attribute is declared using an unrecognized (foreign) type.
		// If so, declaration needs to be direct, bypassing the normal ompp attribute mechanisms.
		// This allows Modgen 'data members', which can be attributes like
		//   vector_double_type breast_cancer_hazard;
		// where
		//   typedef std::vector<double> vector_double_type;
		//
		// Entity members with a foreign type do not participate
		// in the ompp mechanisms of derived attributes, identity attributes,
		// events, and tabulation.

		auto typ = dynamic_cast<TypeSymbol *> (pp_symbol(data_type));
		assert(typ); // parser guarantee
		bool is_foreign = typ->is_foreign();

		if (is_foreign) {
			// Morph to foreign data member.
			// This will change (in the symbol table) the symbol associated with this name
			// from a SimpleAttributeSymbol to an EntityForeignMemberSymbol
			auto sym = new EntityForeignMemberSymbol(this, agent, data_type, decl_loc);
		}

		break;
	}
	default:
		break;
	}
}
