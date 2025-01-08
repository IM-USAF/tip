
#include "tmats_parser.h"

CodeName::CodeName(std::string raw, bool show_debug)
{
    debug = show_debug;
    plaintext = raw;
    std::smatch match;
    std::string prefix;
    std::string match_substr;
    if (debug)
        printf("\n\nCodeName::CodeName(): raw = %s\n\n", raw.c_str());
    while (regex_search(raw, match, (std::regex) "([0-9A-Z\\\\-]*)([a-z]+)"))
    {
        prefix = match.prefix();
        match_substr = match.str(1);
        regex_string += prefix + match_substr;
        regex_string += "([0-9]+)";
        if (debug)
        {
            printf(
                "raw %s, prefix %s, match_substr %s, regex_string %s, "
                "subattr %s\n",
                raw.c_str(), prefix.c_str(), match_substr.c_str(),
                regex_string.c_str(), match.str(2).c_str());
        }
        subattrs.push_back(match.str(2));
        raw = match.suffix().str();
    }

    // TEST
    // If the suffix is not empty, then the suffix wasn't matched
    // to the regex and therefore includes non-variable values
    // which are simply part of the code. 
    if (raw.length() > 0 && regex_string.length() > 0)
        regex_string += raw;

    // Find and escape slashes before building the regex.
    std::string escaped;
    for (int i = 0; i < regex_string.length(); i++)
    {
        if (regex_string[i] == '\\')
            escaped += "\\\\";
        else
            escaped += regex_string[i];
    }

    regex_string = escaped;
    re = (std::regex)escaped;
    if (debug)
        printf("\nescaped regex_string %s\n", regex_string.c_str());
}

// Take a regex match (from CodeName->re) and parse out the subattrs (x, n, etc.).
std::map<std::string, int> CodeName::groups(std::smatch match)
{
    std::map<std::string, int> result;
    for (int i = 0; i < subattrs.size(); i++)
        result[subattrs[i]] = stoi(match.str(i + 1));
    return result;
}

TMATSParser::TMATSParser(const std::string& tmats, bool show_debug)
{
    raw = tmats;
    debug = show_debug;
}

bool TMATSParser::MapAttrsHelper(std::string key_attr, std::string value_attr, 
    std::vector<subattr_data_tuple>& keys,
    std::vector<subattr_data_tuple>& values)
{
    if(!ParseLines(key_attr, value_attr, keys, values))
        return false;

    if (debug)
    {
        IterableTools iter;
        std::vector<std::string> cols({"label", "value"});
        std::string map_name;
        char buff[100];
        std::map<std::string, int> temp_map;
        printf("\nKeys map (%s):\n", key_attr.c_str());
        for (std::vector<subattr_data_tuple>::const_iterator it =
                 keys.begin();
             it != keys.end(); ++it)
        {
            snprintf(buff, std::get<1>(*it).size() + 17, "mapped-to value: %s", std::get<1>(*it).c_str());
            map_name = std::string(buff);
            temp_map = std::get<0>(*it);
            iter.PrintMapWithHeader_KeyToValue(temp_map, cols, map_name);
        }

        printf("\nValues map (%s):\n", value_attr.c_str());
        for (std::vector<subattr_data_tuple>::const_iterator it =
                 values.begin();
             it != values.end(); ++it)
        {
            snprintf(buff, std::get<1>(*it).size() + 17, "mapped-to value: %s", std::get<1>(*it).c_str());
            map_name = std::string(buff);
            temp_map = std::get<0>(*it);
            iter.PrintMapWithHeader_KeyToValue(temp_map, cols, map_name);
        }
    }
    return true;
}

// Takes two TMATS attribute codenames (as seen in the Chapter 9 standard) and
// returns matching values as key:value pairs. ie: map_attrs("R-n", "G-n")
// returns the values of all found pairs of R-n:G-n where n is some number.
bool TMATSParser::MapAttrs(const std::string& key_attr, const std::string& value_attr, 
    std::map<std::string, std::string>& mapped)
{
    std::vector<subattr_data_tuple> keys;
    std::vector<subattr_data_tuple> values;
    if(!MapAttrsHelper(key_attr, value_attr, keys, values))
        return false;

    // Connect keys and values
    if(!MapKeyToValue(keys, values, mapped))
    {
        printf("TMATSParser::MapAttrs: failed to map \"%s\", \"%s\"\n",
            key_attr.c_str(), value_attr.c_str());
        return false;
    }
    return true;
}

bool TMATSParser::MapAttrs(const std::string& key_attr, const std::string& value_attr, 
    std::map<std::string, std::vector<std::string>>& mapped)
{
    std::vector<subattr_data_tuple> keys;
    std::vector<subattr_data_tuple> values;
    if(!MapAttrsHelper(key_attr, value_attr, keys, values))
        return false;

    // Connect keys and values
    if(!MapKeyToValue(keys, values, mapped))
    {
        printf("TMATSParser::MapAttrs: failed to map \"%s\", \"%s\"\n",
            key_attr.c_str(), value_attr.c_str());
        return false;
    }
    return true;
}

bool TMATSParser::MapAttrs(const std::string& key_attr, const std::string& value_attr, 
    std::map<std::string, std::map<std::string, std::string>>& mapped)
{
    std::vector<subattr_data_tuple> keys;
    std::vector<subattr_data_tuple> values;
    if(!MapAttrsHelper(key_attr, value_attr, keys, values))
        return false;

    // Connect keys and values
    if(!MapKeyToValue(keys, values, mapped))
    {
        printf("TMATSParser::MapAttrs: failed to map \"%s\", \"%s\"\n",
            key_attr.c_str(), value_attr.c_str());
        return false;
    }
    return true;
}


bool TMATSParser::MapUnilateralAttrs(const std::set<std::string>& key_attrs, 
    unilateral_map& uni_map)
{
    bool retval = true;
    for(std::set<std::string>::const_iterator it = key_attrs.cbegin(); 
        it != key_attrs.cend(); it++)
    {
        subattr_map index_to_val_map;
        if (!ParseLines(*it, index_to_val_map))
        {
            printf("TMATSParser::MapUnilateralAttrs: Failed to generate "
                "code name regex for %s\n", it->c_str());
            retval = false;
        }
        for(subattr_map::const_iterator it2 = index_to_val_map.cbegin();
            it2 != index_to_val_map.cend(); it2++)
        {
            if (uni_map.count(it2->first) == 0)
            {
                std::map<std::string, std::string> code_to_val_map{{*it, 
                    it2->second}};
                uni_map[it2->first] = code_to_val_map;
            }
            else
            {
                uni_map.at(it2->first)[*it] = it2->second;
            }
        }
    }
    return retval;
}

bool TMATSParser::ParsePCMF1Data(unilateral_map& uni_map, 
    std::set<std::string> req_attrs, std::set<std::string> opt_attrs)
{
    IterableTools iter;
    std::set<std::string> all_attrs = iter.Union(req_attrs, opt_attrs);
    if (!MapUnilateralAttrs(all_attrs, uni_map))
        return false;

    bool retval = true;
    for (unilateral_map::const_iterator it = uni_map.cbegin();
        it != uni_map.cend(); it++)
    {
        for(std::set<std::string>::const_iterator attr = req_attrs.cbegin();
            attr != req_attrs.cend(); attr++)
        {
            if (!iter.IsKeyInMap(it->second, *attr))
            {
                printf("TMATSParser::ParsePCMF1Data: Required attribute "
                "\"%s\" not found in TMATS matter for PCM attributes "
                "index %d\n", attr->c_str(), it->first);
                retval = false;
            }
        }
    }
    return retval;
}



bool TMATSParser::ParseLines(std::string key_attr, subattr_map& vals)
{
    // WARNING! This is unilateral, as in there is no second code with
    // which matching indices (n,m) should be matched. It simply maps
    // the index value with the right-side (of the colon) value. 
    // Most importantly, the current state of this function doesn't 
    // handle multiple variable inputs such as "R-m\TK4-n". Other
    // ParseLines functions do handle multiple variables, but only
    // in the bilateral matching case, in which a code is matched 
    // to another code by the same values in the indices. 
    CodeName key(key_attr, debug);
    if(key.regex_string == "")
        return false;

    // Find matching keys and values;
    std::smatch data_record_match;
    std::string codename;
    std::smatch code_match;
    std::string s = raw.substr(0, raw.length());
    std::regex r = (std::regex) "([0-9A-Za-z\\\\-]+):(.*);";
    if (debug)
        printf("\n\nTMATSParser::MapAttrs():\n");
    for (std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r);
         i != std::sregex_iterator(); i++)
    {
        data_record_match = *i;
        codename = data_record_match.str(1);

        if (debug)
            printf("\nbasic match codename: %s\n", codename.c_str());

        // Get the value of the key attribute and its subattrs (x, n, etc.).
        if (regex_search(codename, code_match, key.re))
        {
            subattr_data_tuple temp_tuple(key.groups(code_match), data_record_match.str(2));
            vals[std::get<0>(temp_tuple).begin()->second] = std::get<1>(temp_tuple);
        }
    }
    return true;
}

bool TMATSParser::ParseLines(std::string key_attr, std::string value_attr, 
    std::vector<subattr_data_tuple>& keys,
    std::vector<subattr_data_tuple>& values)
{
    CodeName key(key_attr, debug);
    if(key.regex_string == "")
        return false;
    CodeName value(value_attr, debug);
    if(value.regex_string == "")
        return false;

    // Find matching keys and values;
    std::smatch data_record_match;
    std::string codename;
    std::smatch code_match;
    std::string s = raw.substr(0, raw.length());
    std::regex r = (std::regex) "([0-9A-Za-z\\\\-]+):(.*);";
    if (debug)
        printf("\n\nTMATSParser::MapAttrs():\n");
    for (std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r);
         i != std::sregex_iterator(); i++)
    {
        data_record_match = *i;
        codename = data_record_match.str(1);

        if (debug)
            printf("\nbasic match codename: %s\n", codename.c_str());

        // Get the value of the key attribute and its subattrs (x, n, etc.).
        if (regex_search(codename, code_match, key.re))
        {
            subattr_data_tuple temp_tuple(key.groups(code_match), data_record_match.str(2));
            keys.push_back(temp_tuple);
            if (debug)
                printf("KEY match (%s): %s\n", key_attr.c_str(), data_record_match.str(2).c_str());
        }
        else if (regex_search(codename, code_match, value.re))
        {
            subattr_data_tuple temp_tuple(value.groups(code_match), data_record_match.str(2));
            values.push_back(temp_tuple);
            if (debug)
                printf("VALUE match (%s): %s\n", value_attr.c_str(), data_record_match.str(2).c_str());
        }
    }
    return true;
}

bool TMATSParser::CheckVarCountEquality(const std::vector<subattr_data_tuple>& keys, 
    const std::vector<subattr_data_tuple>& values, size_t adjustment)
{
    if(keys.size() == 0)
    {
        printf("TMATSParser::CheckVarCountEquality: keys has size zero\n");
        return false;
    }

    if(values.size() == 0)
    {
        printf("TMATSParser::CheckVarCountEquality: values has size zero\n");
        return false;
    }

    size_t var_count = std::get<0>(keys.at(0)).size();
    if(!CheckVarCount(keys, var_count))
    {
        printf("TMATSParser::CheckVarCountEquality: keys not consistent with var_count = %zu\n",
            var_count);
        return false;
    }

    if(!CheckVarCount(values, var_count + adjustment))
    {
        printf("TMATSParser::CheckVarCountEquality: values not consistent with var_count = %zu\n",
            var_count);
        return false;
    }
   
    return true;
}


bool TMATSParser::MapKeyToValue(const std::vector<subattr_data_tuple>& keys, 
    const std::vector<subattr_data_tuple>& values, std::map<std::string, std::string>& mapped)
{
    if(!CheckVarCountEquality(keys, values))
    {
        printf("TMATSParser::MapKeyToValue: var count not equal\n");
        return false;
    }

    std::vector<subattr_data_tuple>::const_iterator key_it;
    std::vector<subattr_data_tuple>::const_iterator val_it;
    std::string key_data;
    subattr_index_map key_map;
    subattr_index_map val_map;
    subattr_index_map::const_iterator var_it;
    bool var_match = false;

    for(key_it = keys.cbegin(); key_it != keys.cend(); ++key_it)
    {
        key_map = std::get<0>(*key_it);
        key_data = std::get<1>(*key_it);
        for(val_it = values.cbegin(); val_it != values.cend(); ++val_it)
        {
            var_match = true;
            val_map = std::get<0>(*val_it);
            for(var_it = key_map.cbegin(); var_it != key_map.cend(); ++var_it)
            {
                if(val_map.count(var_it->first) == 0)
                {
                    printf("Values index map does not have var %s\n", var_it->first.c_str());
                    mapped.clear();
                    return false;
                }

                if(var_it->second != val_map.at(var_it->first))
                {
                    var_match = false;
                    break;
                }
            }

            if(var_match)
            {
                mapped[key_data] = std::get<1>(*val_it);
                break;
            }
        }

        // if(!var_match)
        // {
        //     printf("Failed to map key with data: %s\n", key_data.c_str());
        //     mapped.clear();
        //     return false;
        // }
    }

    return true;
}

bool TMATSParser::MapKeyToValue(const std::vector<subattr_data_tuple>& keys, 
    const std::vector<subattr_data_tuple>& values, 
    std::map<std::string, std::vector<std::string>>& mapped)
{
    if(!CheckVarCountEquality(keys, values, 1))
    {
        printf("TMATSParser::MapKeyToValue: values var count not equal to key var count + 1\n");
        return false;
    }

    std::vector<subattr_data_tuple>::const_iterator key_it;
    std::vector<subattr_data_tuple>::const_iterator val_it;
    std::string key_data;
    subattr_index_map key_map;
    subattr_index_map val_map;
    subattr_index_map::const_iterator var_it;
    bool var_match = false;
    bool match = false;

    for(key_it = keys.cbegin(); key_it != keys.cend(); ++key_it)
    {
        key_map = std::get<0>(*key_it);
        key_data = std::get<1>(*key_it);
        std::vector<std::string> temp_vec;
        match = false;
        for(val_it = values.cbegin(); val_it != values.cend(); ++val_it)
        {
            var_match = true;
            val_map = std::get<0>(*val_it);
            for(var_it = key_map.cbegin(); var_it != key_map.cend(); ++var_it)
            {
                if(val_map.count(var_it->first) == 0)
                {
                    printf("Values index map does not have var %s\n", var_it->first.c_str());
                    mapped.clear();
                    return false;
                }

                if(var_it->second != val_map.at(var_it->first))
                {
                    var_match = false;
                    break;
                }
            }

            if(var_match)
            {
                temp_vec.push_back(std::get<1>(*val_it));
                match = true;
            }
        }

        if(match)
            mapped[key_data] = temp_vec;
    }
   return true;
}

bool TMATSParser::MapKeyToValue(const std::vector<subattr_data_tuple>& keys, 
    const std::vector<subattr_data_tuple>& values, 
    std::map<std::string, std::map<std::string, std::string>>& mapped)
{
    if(!CheckVarCountEquality(keys, values, 1))
    {
        printf("TMATSParser::MapKeyToValue: values var count not equal to key var count + 1\n");
        return false;
    }

    std::string indep_var = GetIndependentVarName(keys, values);

    std::vector<subattr_data_tuple>::const_iterator key_it;
    std::vector<subattr_data_tuple>::const_iterator val_it;
    std::string key_data;
    subattr_index_map key_map;
    subattr_index_map val_map;
    subattr_index_map::const_iterator var_it;
    bool var_match = false;
    bool match = false;
    std::string indep_var_value;

    for(key_it = keys.cbegin(); key_it != keys.cend(); ++key_it)
    {
        key_map = std::get<0>(*key_it);
        key_data = std::get<1>(*key_it);
        std::map<std::string, std::string> temp_map;
        match = false;
        for(val_it = values.cbegin(); val_it != values.cend(); ++val_it)
        {
            var_match = true;
            val_map = std::get<0>(*val_it);

            for(var_it = val_map.cbegin(); var_it != val_map.cend(); ++var_it)
            {
                if(var_it->first == indep_var)
                {
                    indep_var_value = std::to_string(var_it->second);
                    continue;
                }

                if(key_map.count(var_it->first) == 0)
                {
                    printf("Keys index map does not have var %s\n", var_it->first.c_str());
                    mapped.clear();
                    return false;
                }

                if(var_it->second != key_map.at(var_it->first))
                {
                    var_match = false;
                    break;
                }
            }

            if(var_match)
            {
                temp_map[indep_var_value] = std::get<1>(*val_it);
                match = true;
            }
        }

        // if(!match)
        // {
        //     printf("Failed to map key with data: %s\n", key_data.c_str());
        //     mapped.clear();
        //     return false;
        // }
        if(match)
            mapped[key_data] = temp_map;
    }
   return true;

}

std::string TMATSParser::GetIndependentVarName(const std::vector<subattr_data_tuple>& keys, 
    const std::vector<subattr_data_tuple>& values)
{
    std::string var_name = "";
    subattr_index_map key_map = std::get<0>(keys.at(0));
    subattr_index_map val_map = std::get<0>(values.at(0));
    for(subattr_index_map::const_iterator it = val_map.cbegin(); it != val_map.cend();
        ++it)
    {
        if(key_map.count(it->first) == 0)
        {
            var_name = it->first;
            return var_name;
        }
    }
    return var_name;
}


bool TMATSParser::CheckVarCount(const std::vector<subattr_data_tuple>& tuples, size_t count)
{
    std::vector<subattr_data_tuple>::const_iterator it;
    for(it = tuples.cbegin(); it != tuples.cend(); ++it)
    {
        const subattr_index_map& index_map = std::get<0>(*it);
        if(index_map.size() != count)
            return false;
    }
    return true;
}

