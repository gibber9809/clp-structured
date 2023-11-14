#include "EncodedVariableInterpreter.hpp"

// C++ standard libraries
#include <cmath>

#include <cassert>

// spdlog
#include "submodules/spdlog/include/spdlog/spdlog.h"

// Project headers
#include "Defs.hpp"
#include "src/clp_structured/VariableEncoder.hpp"
#include "type_utils.hpp"

// Bad temporary includes
#include "TmpUtils.hpp"

using std::string;
using std::unordered_set;
using std::vector;

namespace clp_structured {
bool EncodedVariableInterpreter::encode_and_search_dictionary(
        string const& var_str,
        VariableDictionaryReader const& var_dict,
        bool ignore_case,
        string& logtype,
        SubQuery& sub_query
) {
    size_t length = var_str.length();
    if (0 == length) {
        throw OperationFailed(ErrorCodeBadParam, __FILENAME__, __LINE__);
    }

    encoded_variable_t encoded_var;
    if (VariableEncoder::convert_string_to_representable_integer_var(var_str, encoded_var)) {
        LogTypeDictionaryEntry::add_non_double_var(logtype);
        sub_query.add_non_dict_var(encoded_var);
    } else if (VariableEncoder::convert_string_to_representable_double_var(var_str, encoded_var)) {
        LogTypeDictionaryEntry::add_double_var(logtype);
        sub_query.add_non_dict_var(encoded_var);
    } else {
        auto entry = var_dict.get_entry_matching_value(var_str, ignore_case);
        if (nullptr == entry) {
            // Not in dictionary
            return false;
        }
        encoded_var = VariableEncoder::encode_var_dict_id(entry->get_id());

        LogTypeDictionaryEntry::add_non_double_var(logtype);
        sub_query.add_dict_var(encoded_var, entry);
    }

    return true;
}

bool EncodedVariableInterpreter::wildcard_search_dictionary_and_get_encoded_matches(
        std::string const& var_wildcard_str,
        VariableDictionaryReader const& var_dict,
        bool ignore_case,
        SubQuery& sub_query
) {
    // Find matches
    unordered_set<VariableDictionaryEntry const*> var_dict_entries;
    var_dict.get_entries_matching_wildcard_string(var_wildcard_str, ignore_case, var_dict_entries);
    if (var_dict_entries.empty()) {
        // Not in dictionary
        return false;
    }

    // Encode matches
    unordered_set<encoded_variable_t> encoded_vars;
    for (auto entry : var_dict_entries) {
        encoded_vars.insert(VariableEncoder::encode_var_dict_id(entry->get_id()));
    }

    sub_query.add_imprecise_dict_var(encoded_vars, var_dict_entries);

    return true;
}
}  // namespace clp_structured
