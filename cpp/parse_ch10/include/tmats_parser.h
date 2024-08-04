#ifndef TMATS_PARSER_H_
#define TMATS_PARSER_H_

#include <fstream>
#include <map>
#include <tuple>
#include <string>
#include <regex>
#include <iostream>
#include <cstdio>
#include <vector>
#include "iterable_tools.h"

using subattr_map = std::map<int, std::string>;
// {index, {code name, value}}
using unilateral_map = std::map<int, std::map<std::string, std::string>>;
using subattr_index_map = std::map<std::string, int>;
using subattr_data_tuple = std::tuple<subattr_index_map, std::string>;

class CodeName
{
   private:
    std::vector<std::string> subattrs;

   public:
    std::string plaintext;
    std::regex re;
    std::string regex_string;
    bool debug;
    CodeName(std::string, bool show_debug = false);
    std::map<std::string, int> groups(std::smatch);
};

class TMATSParser
{
   private:
    std::string raw;
    bool debug;

   public:
    TMATSParser(const std::string&, bool show_debug = false);
    virtual ~TMATSParser() {}

    /*
    Map the values of two TMATS attributes.

    Args:
        key_attr    --> String of TMATs generic data code to
                        be matched as the key
        value_attr  --> String of TMATs generic data code to
                        be matched as the value
        mapped      --> Mapped data

    Return:
        False if the data can't be mapped; true otherwise.
    */
    bool MapAttrs(const std::string& key_attr, const std::string& value_attr, 
        std::map<std::string, std::string>& mapped);
    bool MapAttrs(const std::string& key_attr, const std::string& value_attr, 
        std::map<std::string, std::vector<std::string>>& mapped);
    bool MapAttrs(const std::string& key_attr, const std::string& value_attr, 
        std::map<std::string, std::map<std::string, std::string>>& mapped);
   

    //////////////////////////////////////////////////////////////////////////
    //                  internal functions
    //////////////////////////////////////////////////////////////////////////


    /*
    Parse TMATs lines in search for unilateral keys and compile a 
    master map which links the common index to a map of key attribute
    name to string value.

    Args:
        key_attrs   --> Vector of string key attributes, i.e., 
                        "P-d\DN", etc. 
        uni_map     --> Unilateral map to be populated. 

    If a key attribute can't be found in the TMATS data, then
    populate the value mapped to the key with "null". 
    */
    void MapUnilateralAttrs(const std::vector<std::string>& key_attrs, 
        unilateral_map& uni_map);

    /*
    Create CodeName instances for key and value, parse all TMATS data record
    lines and fill vectors of subattr_data_tuple for key and value.

    Args:
        key_attr    --> String of TMATs generic data code to
                        be matched as the key
        value_attr  --> String of TMATs generic data code to
                        be matched as the value
        keys        --> Vector of key subattr_data_tuple
        values      --> Vector of value subattr_data_tuple

    Return:
        False if regex string is empty; true otherwise.
    */
    bool ParseLines(std::string key_attr, std::string value_attr, 
        std::vector<subattr_data_tuple>& keys,
        std::vector<subattr_data_tuple>& values);
        

    /*
    Create CodeName instances for key, parse all TMATS data record
    lines and fill vectors of subattr_map for key.

    Args:
        key_attr    --> String of TMATs generic data code to
                        be matched as the key
        vals        --> Populated subattr_map of index to 
                        mapped value for the given key_attr

        Ex:
        P-1\F1:16;
        P-2\F1:20;

        ParseLines("P-d\\F1", vals);
        vals --> {{"1", "16"}, {"2", "20"}}

    Return:
        False if regex string is empty; true otherwise.
    */
    bool ParseLines(std::string key_attr, subattr_map& vals);

    
    /*
    Helper function to simplify composition of multiple
    MapAttrs overloads.

    Args:
        key_attr    --> String of TMATs generic data code to
                        be matched as the key
        value_attr  --> String of TMATs generic data code to
                        be matched as the value
        keys        --> Vector of key subattr_data_tuple
        values      --> Vector of value subattr_data_tuple

    Return:
        False if ParseLines returns false.
    */
    bool MapAttrsHelper(std::string key_attr, std::string value_attr, 
        std::vector<subattr_data_tuple>& keys,
        std::vector<subattr_data_tuple>& values);



    /*
    Check variable count in the map component of
    each subattr_data_tuple in the vector.

    TODO: Need to check that all subattr_index_maps have
    the same var names? Ex: x, n, m for all in addition
    to count of 3?

    Args:
        tuples      --> Vector of subattr_data_tuple
        count       --> Expected count in the map of each tuple

    Return:
        True if all maps in the vector of tuples has 
        count values; false otherwise.
    */
    bool CheckVarCount(const std::vector<subattr_data_tuple>& tuples, size_t count);



    /*
    Check var count equality. Use first subattr_data_tuple in the keys
    vector as the expected var count. Check key and value vectors for
    this count of vars.

    Args:
        keys        --> Vector of key subattr_data_tuple
        values      --> Vector of value subattr_data_tuple
        adjustment  --> Add adjustment to expected var_count
                        for values only.

    Return:
        False if vectors are empty or var count is not consistent
        with the first key entry in the keys vector or the values
        vector. True otherwise.
    */
    bool CheckVarCountEquality(const std::vector<subattr_data_tuple>& keys, 
        const std::vector<subattr_data_tuple>& values, size_t adjustment=0);


    
    /*
    From a keys vector and a values vector of subattr_data_tuples, find
    the name of the variable in the values vector which is not in the keys
    vector. Assume it is confirmed that the values tuples have count + 1 vars
    where the keys tuples have count vars. Use CheckVarCountEquality to 
    qualify the vectors for use with this function (adjustment = 1).

    Args:
        keys        --> Vector of key subattr_data_tuple
        values      --> Vector of value subattr_data_tuple

    Return:
        String name of the independent variable 
    */
    std::string GetIndependentVarName(const std::vector<subattr_data_tuple>& keys, 
        const std::vector<subattr_data_tuple>& values);



    /*
    Map single key to single value from subattr_data_tuples.

    Ex: R-1\ID:DATASOURCE; (R-x\ID)
        V-1\ID:DATASOURCE; (V-x\ID)
    get<0>(keys) = <x, 1>
    get<0>(values) = <x, 1>
    get<1>(keys) = DATASOURCE
    get<1>(values) = DATASOURCE
    mapped = <DATASOURCE, DATASOURCE>

    Also, for multiple variables (x, n) in the case keys
    and values have the same count and labels of variables.

    Ex: R-1\TK1-1:1;      (R-x\TK1-n)
        R-1\CDT-1:TIMEIN; (R-x\CDT-n)
    mapped = <1, TIMEIN>

    Args:
        keys        --> Vector of key subattr_data_tuple
        values      --> Vector of value subattr_data_tuple
        mapped      --> Output map

    Return:
        False if failed to match all attributes; true otherwise.
    */
    bool MapKeyToValue(const std::vector<subattr_data_tuple>& keys, 
        const std::vector<subattr_data_tuple>& values, std::map<std::string, std::string>& mapped);



    /*
    Map key to vector of values. The values vector must have count + 1 variables
    where count is true for CheckVarCount(keys, count). The last var is wildcard.

    Ex: R-1\TK1-1:4;      (R-x\TK1-n)
        R-1\ASN-1-1:22;   (R-x\ASN-n-m)
        R-1\ASN-1-2:23;

    mapped = <4, <22, 23>> 

    Args:
        keys        --> Vector of key subattr_data_tuple
        values      --> Vector of value subattr_data_tuple
        mapped      --> Output map

    Return:
        False if failed to match all attributes; true otherwise.
    */
    bool MapKeyToValue(const std::vector<subattr_data_tuple>& keys, 
        const std::vector<subattr_data_tuple>& values, 
        std::map<std::string, std::vector<std::string>>& mapped);



    /*
    Map key to map of key-value pairs. The values vector must have count + 1
    variables where count is true for CheckVarCount(keys, count). The last var is
    wildcard/independent variable that is used as the key in the secondary map.

    Ex: R-1\TK1-1:4;      (R-x\TK1-n)
        R-1\ASN-1-1:22;   (R-x\ASN-n-m)
        R-1\ASN-1-2:23;

    mapped = <4, <<1, 22>, <2, 23>>>

    Args:
        keys        --> Vector of key subattr_data_tuple
        values      --> Vector of value subattr_data_tuple
        mapped      --> Output map

    Return:
        False if failed to match all attributes; true otherwise.
    */
    bool MapKeyToValue(const std::vector<subattr_data_tuple>& keys, 
        const std::vector<subattr_data_tuple>& values, 
        std::map<std::string, std::map<std::string, std::string>>& mapped);

};

#endif  // TMATS_PARSER_H_
