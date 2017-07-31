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
		// Foreign types are not definitively known until this post-parse pass.
		//
		// If this SimpleAttributeSymbol was declared using an unknown (foreign) type,
		// C++ declaration of the member needs to be direct,
		// bypassing the normal ompp attribute mechanisms.
		//
		// Entity members with a foreign type do not participate
		// in the ompp mechanisms of derived attributes, identity attributes,
		// events, and tabulation.
		//
		// These are Modgen 'data members', e.g.
		//   vector_double_type breast_cancer_hazard;
		// where
		//   typedef std::vector<double> vector_double_type;
		//
		// In the code below, the original SimpleAttributeSymbol created during parsing
		// is replaced by an EntityForeignMemberSymbol with the same name.

		auto typ = dynamic_cast<TypeSymbol *> (pp_symbol(data_type));
		assert(typ); // parser guarantee
		bool is_unknown = typ->is_unknown();

		if (is_unknown) {
			// Note the names of the associated side-effect and notify functions
			// created when this SimpleAttributeSymbol was created.
			// They will be destroyed below.
			auto se = side_effects_fn;
			auto ntfy = notify_fn;

			// Morph this SimpleAttributeSymbol to an EntityForeignMemberSymbol.
			// Morphing changes (in the symbol table) the symbol associated with this name.
			auto sym = new EntityForeignMemberSymbol(this, agent, data_type, decl_loc);
			// Push the name into the post parse ignore hash for the current pass.
			pp_symbols_ignore.insert(sym->unique_name);

			// Remove the side-effect and notify functions from the symbol table
			symbols.erase(se->unique_name);
			symbols.erase(ntfy->unique_name);

			// Ignore them in the current post-parse pass
			pp_symbols_ignore.insert(se->unique_name);
			pp_symbols_ignore.insert(ntfy->unique_name);

			// Delete them
			delete se;
			delete ntfy;
		}

		break;
	}
	default:
		break;
	}
}
