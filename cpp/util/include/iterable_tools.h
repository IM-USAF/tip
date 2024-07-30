#ifndef ITERABLE_TOOLS_H
#define ITERABLE_TOOLS_H

#include <iterator>
#include <algorithm>
#include <numeric>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <sstream>
#include <string>
#include "yaml-cpp/yaml.h"

class IterableTools
{
   public:
    IterableTools();
    ~IterableTools();

    // Note Argument T must be an iterable which has a ::const_iterator member.
    // The title is printed followed by a colon and a newline. The format_spec is
    // used to format the data element (ex: %s, %03d, %3.2f, %e, etc.) and the
    // delimiter string is used to separate elements (ex: "\n" or ", " or "," or "|", etc.).
    // Total example:
    // GetIterablePrintString(vec<int>({1, 2, 3, 4, 5}), "my vec", "%02d", ", ")
    // gives:
    // "my vec:
    // 01, 02, 03, 04, 05
    // "
    template <typename T>
    std::string GetIterablePrintString(const T& input_iterable, std::string title,
                                       std::string format_spec, std::string delim) const;

    // Sort in ascending order.
    // Input iterable must have .begin() and .end() methods to iterators.
    // Note that there is no reason to pass a set because sets are sorted
    // by defintion.
    template <typename T>
    std::vector<T> Sort(const std::vector<T>& input_iterable) const;

    // Requires that input object has .begin() and .end() methods to obtain
    // iterators. Does not require that set objects be used, although those
    // are accepted as well.
    template <typename T>
    std::vector<T> Intersection(const std::vector<T>& input_iterable1,
                                const std::vector<T>& input_iterable2) const;
    template <typename T>
    std::set<T> Intersection(const std::set<T>& input_iterable1,
                             const std::set<T>& input_iterable2) const;

    // Find the union of two vectors or sets.
    template <typename T>
    std::vector<T> Union(const std::vector<T>& input_iterable1,
                         const std::vector<T>& input_iterable2) const;
    template <typename T>
    std::set<T> Union(const std::set<T>& input_iterable1,
                      const std::set<T>& input_iterable2) const;

    // If the first input iterable is a subset of the second input iterable,
    // return true, otherwise return false.
    template <typename T>
    bool IsSubset(const std::vector<T>& input1, const std::vector<T>& input2) const;
    template <typename T>
    bool IsSubset(const std::set<T>& input1, const std::set<T>& input2) const;

    // Pass by value to avoid copying the vector explictly in
    // the function so the original order is not modified by
    // the sort function.
    // According to https://stackoverflow.com/questions/47537049/what-is-the-best-way-to-sort-a-vector-leaving-the-original-one-unaltered
    // the vector move function will be called on return to prevent copying
    // the whole thing again.
    template <typename T>
    std::vector<T> UniqueElements(std::vector<T> input) const;

    // Get a vector of the field values of each element in input.
    template <class C, typename F>
    std::vector<F> VectorOfMember(const std::vector<C>& input, const F(C::*field)) const;

    // Return a vector of the indices of elements in the input vector
    // that sorts the value of the given indices in ascending order.
    // the ArgSort functions are only relevant for vectors of types
    // which can be compared via the '<' operator.
    template <typename T>
    std::vector<size_t> ArgSortAscending(const std::vector<T>& input) const;

    template <typename T>
    std::vector<size_t> ArgSortDescending(const std::vector<T>& input) const;

    // Get a vector of the keys of the map that sort the associated
    // values in ascending order of their length/count.
    // The mapped values must be iterables with a .size() member function.
    template <typename Key, typename Val>
    std::vector<Key> SortMapToIterableByValueSize(const std::map<Key, Val>& input_map) const; 
    template <typename Key, typename Val>
    std::vector<Key> SortMapToIterableByValueSize(const std::unordered_map<Key, Val>& input_map) const; 

    // Find the indices of matching elements. Not efficient for determining
    // if a value is present (Use IndexOf . . . (TODO)). Use to locate
    // indices of matching elements when it is already known or expected to
    // contain the matching value.
    template <typename T>
    std::vector<size_t> IndicesOfMatching(const std::vector<T>& input, const T& match) const; 

    // Swap the map key-value pairs. For each key-value pair, use the initial
    // value as the new key and the initial key as the new value.
    // Note that it's possible to lose entries. Keys are unique but mapped
    // values are not. If a value has already been used as key, an attempt
    // to insert the value as a key again will overwrite the first entry.
    // If this occurs, lost_entries will be set to true, otherwise it is
    // set to false.
    template <typename Key, typename Val>
    std::map<Val, Key> ReverseMap(const std::map<Key, Val>& input_map, bool& lost_entries) const;
    template <typename Key, typename Val>
    std::unordered_map<Val, Key> ReverseMap(const std::unordered_map<Key, Val>& input_map,
                                            bool& lost_entries) const;

    // Return true if the key is in the map, false if not.
    template <typename Key, typename Val>
    bool IsKeyInMap(const std::map<Key, Val>& input_map, const Key& input_key) const;
    template <typename Key, typename Val>
    bool IsKeyInMap(const std::unordered_map<Key, Val>& input_map, const Key& input_key) const;

    // Return a subset of the input vector by selecting elements by indices.
    // Should I rename this to SubsetByIndices or similar?
    template <class C>
    std::vector<C> CollectByIndices(const std::vector<C>& input,
                                    const std::vector<size_t>& inds) const;

    // Group elements of a vector by a common value in the "field" member.
    // The map key is the value of "field" common to all elements in the vector
    // that it maps. Ex: If the input vector contains an object with field "a"
    // and the set of "a" values from all of the objects in the vector have four
    // unique values, then the output map will have four key-value pairs. The four
    // keys will be the unique values of "a" and the mapped vectors will be elements
    // that have the key value as the value of "a."
    template <class C, typename F>
    std::unordered_map<F, std::vector<C>> GroupByMember(const std::vector<C>& input,
                                                        const F(C::*field)) const;

    // Get a vector of the keys in a map.
    template <typename Keys, typename Vals>
    std::vector<Keys> GetKeys(const std::map<Keys, Vals>& input_map) const;
    template <typename Keys, typename Vals>
    std::vector<Keys> GetKeys(const std::unordered_map<Keys, Vals>& input_map) const;

    // Get a vector of the values in a map.
    template <typename Keys, typename Vals>
    std::vector<Vals> GetVals(const std::map<Keys, Vals>& input_map) const;
    template <typename Keys, typename Vals>
    std::vector<Vals> GetVals(const std::unordered_map<Keys, Vals>& input_map) const;

    // Update the keys in a map, leaving the values completed untouched.
    // If the keys in the input map correspond to any of the keys in the
    // update map, replace the key with mapped value in the update map.
    //
    // Ex: Input map = {key:3 --> val:"hi"}, if
    // Update map = {key:3 --> val:10}, set 3 in input map to
    // 10 and return updated map = {key:10 --> val:"hi"}
    template <typename Key, typename Val>
    std::map<Key, Val> UpdateMapKeys(const std::map<Key, Val>& input_map,
                                     const std::map<Key, Key>& update_map) const;
    template <typename Key, typename Val>
    std::unordered_map<Key, Val> UpdateMapKeys(const std::unordered_map<Key, Val>& input_map,
                                               const std::unordered_map<Key, Key>& update_map) const;

    // Update the values in a map, leaving the keys completely untouched.
    // If the values in the input map correspond to any of the keys in the
    // update map, replace the input value with the mapped value in the
    // update map.
    //
    // Ex: Input map = {key:3 --> val:"hi"}, if
    // Update map = {key:"hi" --> val:"test"}, set "hi" in input map to
    // "test" and return updated map = {key:3 --> val:"test"}
    template <typename Key, typename Val>
    std::map<Key, Val> UpdateMapVals(const std::map<Key, Val>& input_map,
                                     const std::map<Val, Val>& update_map) const;
    template <typename Key, typename Val>
    std::unordered_map<Key, Val> UpdateMapVals(const std::unordered_map<Key, Val>& input_map,
                                               const std::unordered_map<Val, Val>& update_map) const;

    // Return a set of keys present in the input map that map to the input
    // match value.
    template <typename Key, typename Val>
    std::set<Key> GetKeysByValue(const std::map<Key, Val>& input_map, const Val& match_val) const;

    template <typename Key, typename Val>
    std::set<Key> GetKeysByValue(const std::unordered_map<Key, Val>& input_map,
                                 const Val& match_val) const;

    // Adds key/values from the update_map to the input_map if
    // the key from the update_map is not in the input_map
    //
    // Ex: input_map = {key:3 --> val:"hi"},
    // update_map = {key:3 --> val:"test"}
    //				{key:4 --> val:"four"}
    // returned map = {key:3 --> val:"hi"}
    //                {key:4 -- > val:"four"}
    template <typename Key, typename Val>
    std::map<Key, Val> CombineMaps(const std::map<Key, Val>& input_map,
                                   const std::map<Key, Val>& update_map) const;

    template <typename Key, typename Val>
    std::unordered_map<Key, Val> CombineMaps(const std::unordered_map<Key, Val>& input_map,
                                             const std::unordered_map<Key, Val>& update_map) const;

    // Adds key/set<value> pairs to the input map if a key found in
    // the update map is not present in the input map. If a key
    // is present in the input map, update the input map value set by
    // inserting any new values in the update map value set. Return
    // a new map by value.
    //
    // Ex: input_map = {{a, {1, 2, 3}}, {c, {2, 3}}}
    //     update_map = {{a, {2, 4}}, {b, {1, 5}}}
    //	   return = {{a, {1, 2, 3, 4}}, {b, {1, 5}}, {c, {2, 3}}}
    template <typename Key, typename Val>
    std::map<Key, std::set<Val>> CombineCompoundMapsToSet(
        const std::map<Key, std::set<Val>>& input_map,
        const std::map<Key, std::set<Val>>& update_map) const;

    // Converts a vector to a set
    //
    template <typename T>
    std::set<T> VecToSet(const std::vector<T>& input_vec) const;

    // Check if two sets are equal regardless of order
    //
    template <typename T>
    bool EqualSets(const std::set<T>& set1, const std::set<T>& set2) const;

    // Deletes the values specified in the deletion set from the original set
    // Returns the new set
    //
    template <typename T>
    std::set<T> DeleteFromSet(const std::set<T>& original, const std::set<T>& deletions) const;

    // Deletes one value from a set
    // Returns the new set
    //
    template <typename T>
    std::set<T> DeleteFromSet(const std::set<T>& original, T deletion) const;

    // Returns the reverse map when the map is a key to a set
    //
    // Ex: input_map = key:3 --> val: {"hi", "four", "cool"},
    //				   key:4 -- > val: {"hi", "step", "cool"},
    // returned map = key: "hi" --> val: {3, 4}
    //                key: "four" --> val: {3}
    //                key: "cool" --> val: {3, 4}
    //                key: "step" --> val: {4}
    template <typename Key, typename Val>
    std::map<Val, std::set<Key>> ReverseMapSet(const std::map<Key, std::set<Val>>& input_map) const;

    // Returns the reverse unordered map when the map is a key to a set
    //
    // Ex: input_map = key:3 --> val: {"hi", "four", "cool"},
    //				   key:4 -- > val: {"hi", "step", "cool"},
    // returned map = key: "hi" --> val: {3, 4}
    //                key: "four" --> val: {3}
    //                key: "cool" --> val: {3, 4}
    //                key: "step" --> val: {4}
    template <typename Key, typename Val>
    std::unordered_map<Val, std::set<Key>> ReverseMapSet(const std::unordered_map<Key, std::set<Val>>& input_map) const;

    // Binary searches the input vector/set and returns index of the first
    // matching element, otherwise return std::string::npos.
    //
    // Note: input vectors must be sorted in ascending order, otherwise the result
    // is invalid. This function is meant to be very fast, so there is no check
    // for sorted data.
    template <typename T>
    size_t IndexOf(const std::vector<T>& input, const T& search_key) const;
    template <typename T>
    size_t IndexOf(const std::set<T>& input, const T& search_key) const;

    // Formats the map elements into a string. The input map can be a key to a set
    // Sorts the list in order of largest to smallest set
    // Returns: the printable string
    //
    // Ex: input_map =  key:3 --> val:{"hi","jack"},
    //                  key:4 --> val:{"sorry","jack","jump"}
    // return value   = 4:    sorry,jack,jump
    //                  3:    hi,jack
    //
    template <typename Key, typename Val>
    std::string GetPrintableMapElements_KeyToSet(std::map<Key, std::set<Val>> input_map) const;

    // Formats the map elements into a string. The input map can be a key to a value
    // Returns: the printable string
    //
    // Ex: input_map =  key:3 --> val:"hi"
    //                  key:4 --> val:"sorry"
    // return value   = 3:    hi
    //                  4:    sorry
    template <typename Key, typename Val>
    std::string GetPrintableMapElements_KeyToValue(const std::map<Key, Val>& input_map) const;

    // Formats the map elements into a string. The input map can be a key to a bool value
    // Returns: the printable string
    //
    // Ex: input_map =  key:3 --> val:true
    //                  key:4 --> val:false
    // return value   = 3:    true
    //                  4:    false
    template <typename Key>
    std::string GetPrintableMapElements_KeyToBool(const std::map<Key, bool>& input_map) const;

    // Prints map with header. The input map can be a key to a set
    // Returns: the printed string
    //
    // Ex: input_map =  key:3 --> val:{"hi","jack"},
    //                  key:4 --> val:{"sorry","jack"}
    //     columns   = {"Col1", "Col2"}
    //     map_name  = "map name"
    //
    // return value  =    map name
    //                   (Col1) | (Col2)
    //                  ---------------------------
    //                   3:    hi,jack
    //                   4:    sorry,jack
    template <typename Key, typename Val>
    std::string PrintMapWithHeader_KeyToSet(const std::map<Key, std::set<Val>>& input_map,
                                            std::vector<std::string> columns, std::string map_name) const;

    // Prints map with header. The input map can be a key to a value
    // Returns: the printed string
    //
    // Ex: input_map =  key:3 --> val:"hi",
    //                  key:4 --> val:"sorry"
    //     columns   = {"Col1", "Col2"}
    //     map_name  = "map name"
    //
    // return value  =    map name
    //                   (Col1) | (Col2)
    //                  ---------------------------
    //                   3:    hi
    //                   4:    sorry
    template <typename Key, typename Val>
    std::string PrintMapWithHeader_KeyToValue(const std::map<Key, Val>& input_map, std::vector<std::string> columns, std::string map_name) const;

    // Prints map with header. The input map can be a key to a value
    // Returns: the printed string
    //
    // Ex: input_map =  key:3 --> val:true,
    //                  key:4 --> val:false
    //     columns   = {"Col1", "Col2"}
    //     map_name  = "map name"
    //
    // return value  =    map name
    //                   (Col1) | (Col2)
    //                  ---------------------------
    //                   3:    true
    //                   4:    false
    template <typename Key>
    std::string PrintMapWithHeader_KeyToBool(const std::map<Key, bool>& input_map, std::vector<std::string> columns, std::string map_name) const;

    // Returns a header with a title, column(s), and header bar
    //
    // Ex: columns   = {"Col1", "Col2"}
    //     map_name  = "map name"
    //
    // return value  =    map name
    //                   (Col1) | (Col2)
    //                  ---------------------------
    std::string GetHeader(std::vector<std::string> columns, std::string title) const;

    // Returns a string with the header/footer bar
    //
    // return value  =   ---------------------------
    std::string GetPrintBar() const;

    // Prints the string and returns the printed string (used for testing purposes)
    //
    std::string print(std::string print_string) const;

    // Note for yaml emitter help functions below:
    // a key type of uint8_t is interpreted as a char and will
    // output a char representation. Avoid use of uint8_t.

    // Emits a single key:value pair as a Yaml map object. This function
    // is not intended to be used multiple times to accomplish emission
    // of a multiple-key/value map. The pair emitted via this function
    // is intended to be a standalone pairing, such as configuration
    // key and value. See EmitKeyValuePairWithinMap for use within
    // a map that has already been created.
    //
    // Inputs:
    //	- YAML::Emitter e: Emitter to which Yaml is emitted
    //	- input_key: Key to be emitted
    //	- input_val: Value to be emitted
    //
    template <typename Key, typename Val>
    void EmitKeyValuePair(YAML::Emitter& e, const Key& input_key,
                          const Val& input_value) const;
    template <typename Key, typename Val>
    void EmitKeyValuePairWithinMap(YAML::Emitter& e, const Key& input_key,
                          const Val& input_value) const;


    // Emits a key:value map to the Yaml emitter passed as an argument.
    //
    // Inputs:
    //	- YAML::Emitter e: Emitter to which Yaml is emitted
    //	- input_map: Key:Value map which is emitted as Yaml
    //	- map_name: name of map to be emitted
    //
    template <typename Key, typename Val>
    void EmitSimpleMap(YAML::Emitter& e, const std::map<Key, Val>& input_map,
                       const std::string& map_name) const;
    template <typename Key, typename Val>
    void EmitSimpleMapWithinMap(YAML::Emitter& e, const std::map<Key, Val>& input_map,
                       const std::string& map_name) const;

    // Emits a key:set<values> map to the Yaml emitter passed as an argument.
    //
    // Inputs:
    //	- YAML::Emitter e: Emitter to which Yaml is emitted
    //	- input_map: Key:set<Value> map which is emitted as Yaml
    //	- map_name: name of map to be emitted
    //
    template <typename Key, typename Val>
    void EmitCompoundMapToSet(YAML::Emitter& e,
                              const std::map<Key, std::set<Val>>& input_map, const std::string& key) const;
    template <typename Key, typename Val>
    void EmitCompoundMapToSetWithinMap(YAML::Emitter& e,
                              const std::map<Key, std::set<Val>>& input_map, const std::string& key) const;


    // Emits a key:vector<values> map to the Yaml emitter passed as an argument.
    //
    // Inputs:
    //	- YAML::Emitter e: Emitter to which Yaml is emitted
    //	- input_map: Key:vector<Value> map which is emitted as Yaml
    //	- map_name: name of map to be emitted
    //
    template <typename Key, typename Val>
    void EmitCompoundMapToVector(YAML::Emitter& e,
                                 const std::map<Key, std::vector<Val>>& input_map, const std::string& key) const;
    template <typename Key, typename Val>
    void EmitCompoundMapToVectorWithinMap(YAML::Emitter& e,
                                 const std::map<Key, std::vector<Val>>& input_map, const std::string& key) const;

    // Emits a sequence<vector<Value>> map to the Yaml emitter passed as an argument.
    // The top-level vector is output in block style and the secondary vector is
    // emitted in flow style.
    //
    // Inputs:
    //	- YAML::Emitter e: Emitter to which Yaml is emitted
    //	- input_vec: vector<vector<Value>> map which is emitted as Yaml
    //
    template <typename Val>
    void EmitSequenceOfVectors(YAML::Emitter& e,
                               const std::vector<std::vector<Val>>& input_vec) const;
};

template <typename T>
std::string IterableTools::GetIterablePrintString(const T& input_iterable, std::string title,
                                                  std::string format_spec, std::string delim) const
{
    int i = 0;
    std::string ret_val = title + ":\n";
    std::string format_and_delim(delim + format_spec);
    char buff[100];
    for (typename T::const_iterator it = input_iterable.begin(); it != input_iterable.end(); ++it)
    {
        if (i == 0)
        {
            snprintf(buff, format_spec.size(), format_spec.c_str(), *it);
        }
        else
        {
            snprintf(buff, format_and_delim.size(), format_and_delim.c_str(), *it);
        }
        ret_val += buff;
        i++;
    }
    ret_val += "\n";
    return ret_val;
}

template <>
std::string IterableTools::GetIterablePrintString<std::vector<std::string>>(
    const std::vector<std::string>& input_iterable, std::string title,
    std::string format_spec, std::string delim) const;

template <typename T>
std::vector<T> IterableTools::Sort(const std::vector<T>& input_iterable) const
{
    std::vector<T> input_sorted(input_iterable.size());
    std::partial_sort_copy(input_iterable.begin(), input_iterable.end(),
                           input_sorted.begin(), input_sorted.end());
    return input_sorted;
}

template <typename T>
std::vector<T> IterableTools::Intersection(const std::vector<T>& input_iterable1,
                                           const std::vector<T>& input_iterable2) const
{
    // Create a sorted version of the inputs.
    std::vector<T> in1_sorted = Sort<T>(input_iterable1);
    std::vector<T> in2_sorted = Sort<T>(input_iterable2);

    // Initialize the output.
    size_t output_size = size_t((in1_sorted.size() > in2_sorted.size())
                                    ? in1_sorted.size()
                                    : in2_sorted.size());
    std::vector<T> output_iterable(output_size);
    typename std::vector<T>::iterator it;

    // Apply the set intersection algorithm.
    it = std::set_intersection(in1_sorted.begin(), in1_sorted.end(),
                               in2_sorted.begin(), in2_sorted.end(), output_iterable.begin());

    // Resize to fit the result.
    output_iterable.resize(it - output_iterable.begin());

    return output_iterable;
}

template <typename T>
std::set<T> IterableTools::Intersection(const std::set<T>& input_iterable1,
                                        const std::set<T>& input_iterable2) const
{
    // Initialize the output.
    std::set<T> output_iterable;

    // Apply the set intersection algorithm.
    std::set_intersection(input_iterable1.begin(), input_iterable1.end(),
                          input_iterable2.begin(), input_iterable2.end(),
                          std::inserter(output_iterable, output_iterable.begin()));

    return output_iterable;
}

template <typename T>
std::vector<T> IterableTools::Union(const std::vector<T>& input_iterable1,
                                    const std::vector<T>& input_iterable2) const
{
    // Create a sorted version of the inputs.
    std::vector<T> in1_sorted = Sort<T>(input_iterable1);
    std::vector<T> in2_sorted = Sort<T>(input_iterable2);

    // Initialize the output.
    size_t output_size = in1_sorted.size() + in2_sorted.size();
    std::vector<T> output_iterable(output_size);
    typename std::vector<T>::iterator it;

    // Apply the set intersection algorithm.
    it = std::set_union(in1_sorted.begin(), in1_sorted.end(),
                        in2_sorted.begin(), in2_sorted.end(), output_iterable.begin());

    // Resize to fit the result.
    output_iterable.resize(it - output_iterable.begin());

    return output_iterable;
}

template <typename T>
std::set<T> IterableTools::Union(const std::set<T>& input_iterable1,
                                 const std::set<T>& input_iterable2) const
{
    // Initialize the output.
    std::set<T> output_iterable;

    // Apply the set intersection algorithm.
    std::set_union(input_iterable1.begin(), input_iterable1.end(),
                   input_iterable2.begin(), input_iterable2.end(),
                   std::inserter(output_iterable, output_iterable.begin()));

    return output_iterable;
}

template <typename T>
std::vector<T> IterableTools::UniqueElements(std::vector<T> input) const
{
    // Sort the elements.
    std::sort(input.begin(), input.end());

    // Find the unique values.
    typename std::vector<T>::iterator it = std::unique(input.begin(), input.end());

    // Resize to include only the unique values.
    input.resize(std::distance(input.begin(), it));

    return input;
}

template <class C, typename F>
std::vector<F> IterableTools::VectorOfMember(const std::vector<C>& input, const F(C::*field)) const
{
    std::vector<F> output(input.size());
    for (size_t i = 0; i < input.size(); i++)
    {
        output[i] = input[i].*field;
    }
    return output;
}

template <typename T>
std::vector<size_t> IterableTools::IndicesOfMatching(const std::vector<T>& input, const T& match) const
{
    std::vector<size_t> inds;
    for (size_t i = 0; i < input.size(); i++)
    {
        if (match == input[i])
            inds.push_back(i);
    }
    return inds;
}

template <class C>
std::vector<C> IterableTools::CollectByIndices(const std::vector<C>& input,
                                               const std::vector<size_t>& inds) const
{
    std::vector<C> output;
    for (size_t i = 0; i < inds.size(); i++)
    {
        output.push_back(input[inds[i]]);
    }
    return output;
}

template <class C, typename F>
std::unordered_map<F, std::vector<C>> IterableTools::GroupByMember(
    const std::vector<C>& input, const F(C::*field)) const
{
    // Get the member values for each element.
    std::vector<F> member_vals = VectorOfMember<C, F>(input, field);

    // Find the unique values of that member.
    std::vector<F> unique_vals = UniqueElements<F>(member_vals);

    // Loop over unique values, find the indices of the original input vector in which
    // the member value matches the current unique value, then collect those objects
    // and place them in the map.
    // TODO(ijmyers): Should we return a map of the unique member values to vectors of indices in
    // the original input vector to save memory?
    std::unordered_map<F, std::vector<C>> output_map;
    std::vector<size_t> indices_of_objects;
    std::vector<C> group_of_objects;
    for (size_t i = 0; i < unique_vals.size(); i++)
    {
        indices_of_objects = IndicesOfMatching<F>(member_vals, unique_vals[i]);
        group_of_objects = CollectByIndices<C>(input, indices_of_objects);
        output_map[unique_vals[i]] = group_of_objects;
    }
    return output_map;
}

template <typename Keys, typename Vals>
std::vector<Keys> IterableTools::GetKeys(const std::map<Keys, Vals>& input_map) const
{
    std::vector<Keys> output_keys;
    for (typename std::map<Keys, Vals>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        output_keys.push_back(it->first);
    }
    return output_keys;
}

template <typename Keys, typename Vals>
std::vector<Keys> IterableTools::GetKeys(const std::unordered_map<Keys, Vals>& input_map) const
{
    std::vector<Keys> output_keys;
    for (typename std::unordered_map<Keys, Vals>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        output_keys.push_back(it->first);
    }
    return output_keys;
}

template <typename Keys, typename Vals>
std::vector<Vals> IterableTools::GetVals(const std::map<Keys, Vals>& input_map) const
{
    std::vector<Vals> output_vals;
    for (typename std::map<Keys, Vals>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        output_vals.push_back(it->second);
    }
    return output_vals;
}

template <typename Keys, typename Vals>
std::vector<Vals> IterableTools::GetVals(const std::unordered_map<Keys, Vals>& input_map) const
{
    std::vector<Vals> output_vals;
    for (typename std::unordered_map<Keys, Vals>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        output_vals.push_back(it->second);
    }
    return output_vals;
}

template <typename T>
bool IterableTools::IsSubset(const std::vector<T>& input1, const std::vector<T>& input2) const
{
    // Sort the inputs.
    std::vector<T> in1_sorted = Sort<T>(input1);
    //std::vector<T> in2_sorted = Sort<std::vector<T>>(input2);

    // Find the intersection of the two inputs.
    std::vector<T> intersect = Intersection<T>(input1, input2);

    // If the intersection has the same count of elements as
    // input1, then input1 is a subset of input2.
    if (input1.size() == intersect.size())
        return true;
    else
        return false;
}

template <typename T>
bool IterableTools::IsSubset(const std::set<T>& input1, const std::set<T>& input2) const
{
    std::set<T> intersect = Intersection<T>(input1, input2);
    if (input1.size() == intersect.size())
        return true;
    else
        return false;
}

template <typename T>
std::vector<size_t> IterableTools::ArgSortAscending(const std::vector<T>& input) const
{
    // Get a vector of an index starting from 0 and
    // incrementing for each element in the input.
    std::vector<size_t> idx(input.size());
    std::iota(idx.begin(), idx.end(), 0);

    // Custom sort the index by the values of input.
    std::sort(idx.begin(), idx.end(),
              [&input](size_t i1, size_t i2) { return input[i1] < input[i2]; });
    return idx;
}

template <typename T>
std::vector<size_t> IterableTools::ArgSortDescending(const std::vector<T>& input) const
{
    // Get a vector of an index starting from 0 and
    // incrementing for each element in the input.
    std::vector<size_t> idx(input.size());
    std::iota(idx.begin(), idx.end(), 0);

    // Custom sort the index by the values of input in descending order.
    std::sort(idx.begin(), idx.end(),
              [&input](size_t i1, size_t i2) { return input[i1] > input[i2]; });
    return idx;
}

template <typename Key, typename Val>
std::vector<Key> IterableTools::SortMapToIterableByValueSize(
    const std::map<Key, Val>& input_map) const
{
    // This implementation is valid for both ordered and unordered maps.

    // Get the keys of this map.
    std::vector<Key> keys = GetKeys<Key, Val>(input_map);

    // Get a second vector with the size/length of the values associated
    // with those keys.
    std::vector<size_t> lengths(keys.size());
    for (int i = 0; i < keys.size(); i++)
    {
        lengths[i] = input_map.at(keys[i]).size();
    }

    // Get the indices of the lengths vector that sorts it
    // in ascending order.
    std::vector<size_t> sort_inds = ArgSortAscending<size_t>(lengths);

    // Build a vector of the keys using the indices that sort by size.
    std::vector<Key> sorted = CollectByIndices<Key>(keys, sort_inds);

    return sorted;
}

template <typename Key, typename Val>
std::vector<Key> IterableTools::SortMapToIterableByValueSize(
    const std::unordered_map<Key, Val>& input_map) const
{
    // This implementation is valid for both ordered and unordered maps.

    // Get the keys of this map.
    std::vector<Key> keys = GetKeys<Key, Val>(input_map);

    // Get a second vector with the size/length of the values associated
    // with those keys.
    std::vector<size_t> lengths(keys.size());
    for (int i = 0; i < keys.size(); i++)
    {
        lengths[i] = input_map[keys[i]].size();
    }

    // Get the indices of the lengths vector that sorts it
    // in ascending order.
    std::vector<size_t> sort_inds = ArgSortAscending<size_t>(lengths);

    // Build a vector of the keys using the indices that sort by size.
    std::vector<Key> sorted = CollectByIndices<Key>(keys, sort_inds);

    return sorted;
}

template <typename Key, typename Val>
std::map<Val, Key> IterableTools::ReverseMap(const std::map<Key, Val>& input_map,
                                             bool& lost_entries) const
{
    lost_entries = false;

    // Get a vector of the keys.
    std::vector<Key> initial_keys = GetKeys<Key, Val>(input_map);

    // Create a set of all of the keys used in the reversed map.
    // If a value has been used as a key in a prior iteration,
    // set lost_entries to true;
    std::set<Val> val_set;

    // Fill a new map with each key-value pair reversed.
    std::map<Val, Key> rev_map;
    for (int i = 0; i < input_map.size(); i++)
    {
        rev_map[input_map.at(initial_keys[i])] = initial_keys[i];
        if (val_set.count(input_map.at(initial_keys[i])) == 1)
            lost_entries = true;
        else
            val_set.insert(input_map.at(initial_keys[i]));
    }
    return rev_map;
}

template <typename Key, typename Val>
std::unordered_map<Val, Key> IterableTools::ReverseMap(
    const std::unordered_map<Key, Val>& input_map, bool& lost_entries) const
{
    lost_entries = false;

    // Get a vector of the keys.
    std::vector<Key> initial_keys = GetKeys<Key, Val>(input_map);

    // Create a set of all of the keys used in the reversed map.
    // If a value has been used as a key in a prior iteration,
    // set lost_entries to true;
    std::set<Val> val_set;

    // Fill a new map with each key-value pair reversed.
    std::unordered_map<Val, Key> rev_map;
    for (int i = 0; i < input_map.size(); i++)
    {
        rev_map[input_map.at(initial_keys[i])] = initial_keys[i];
        if (val_set.count(input_map.at(initial_keys[i])) == 1)
        {
            lost_entries = true;
        }
        else
        {
            val_set.insert(input_map.at(initial_keys[i]));
        }
    }
    return rev_map;
}

template <typename Key, typename Val>
bool IterableTools::IsKeyInMap(const std::map<Key, Val>& input_map,
                               const Key& input_key) const
{
    if (input_map.count(input_key) > 0)
        return true;
    else
        return false;
}

template <typename Key, typename Val>
bool IterableTools::IsKeyInMap(const std::unordered_map<Key, Val>& input_map,
                               const Key& input_key) const
{
    if (input_map.count(input_key) > 0)
        return true;
    else
        return false;
}

template <typename Key, typename Val>
std::map<Key, Val> IterableTools::UpdateMapKeys(const std::map<Key, Val>& input_map,
                                                const std::map<Key, Key>& update_map) const
{
    std::map<Key, Val> new_map;
    // Iterate over the input map. If the current key is in the update map
    // then update the entry with the update map value, otherwise
    // copy the input map key and value.
    Key curr_key;
    for (typename std::map<Key, Val>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        curr_key = it->first;
        if (IsKeyInMap<Key, Key>(update_map, curr_key))
            new_map[update_map.at(curr_key)] = it->second;
        else
            new_map[curr_key] = it->second;
    }
    return new_map;
}

template <typename Key, typename Val>
std::unordered_map<Key, Val> IterableTools::UpdateMapKeys(const std::unordered_map<Key, Val>& input_map,
                                                          const std::unordered_map<Key, Key>& update_map) const
{
    std::unordered_map<Key, Val> new_map;
    // Iterate over the input map. If the current key is in the update map
    // then update the entry with the update map value, otherwise
    // copy the input map key and value.
    Key curr_key;
    for (typename std::unordered_map<Key, Val>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        curr_key = it->first;
        if (IsKeyInMap<Key, Key>(update_map, curr_key))
            new_map[update_map.at(curr_key)] = it->second;
        else
            new_map[curr_key] = it->second;
    }
    return new_map;
}

template <typename Key, typename Val>
std::map<Key, Val> IterableTools::UpdateMapVals(const std::map<Key, Val>& input_map,
                                                const std::map<Val, Val>& update_map) const
{
    std::map<Key, Val> new_map;
    // Iterate over the input map. If the current value is a key in the update map
    // then update the entry with the current input map key and the update map value,
    // otherwise copy the input map key and value.
    Val curr_val;
    for (typename std::map<Key, Val>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        curr_val = it->second;
        if (IsKeyInMap<Val, Val>(update_map, curr_val))
            new_map[it->first] = update_map.at(curr_val);
        else
            new_map[it->first] = curr_val;
    }
    return new_map;
}

template <typename Key, typename Val>
std::unordered_map<Key, Val> IterableTools::UpdateMapVals(
    const std::unordered_map<Key, Val>& input_map,
    const std::unordered_map<Val, Val>& update_map) const
{
    std::unordered_map<Key, Val> new_map;
    // Iterate over the input map. If the current value is a key in the update map
    // then update the entry with the current input map key and the update map value,
    // otherwise copy the input map key and value.
    Val curr_val;
    for (typename std::unordered_map<Key, Val>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        curr_val = it->second;
        if (IsKeyInMap<Val, Val>(update_map, curr_val))
            new_map[it->first] = update_map.at(curr_val);
        else
            new_map[it->first] = curr_val;
    }
    return new_map;
}

template <typename Key, typename Val>
std::map<Key, Val> IterableTools::CombineMaps(const std::map<Key, Val>& input_map, const std::map<Key, Val>& update_map) const
{
    std::map<Key, Val> new_map = input_map;
    // Iterate over the update map
    // Add keys and values to the input map
    // that exist in the update map, but not in the input map
    for (typename std::map<Key, Val>::const_iterator it = update_map.begin(); it != update_map.end(); it++)
    {
        if (new_map.count(it->first) == 0)
            new_map[it->first] = it->second;
    }
    return new_map;
}

template <typename Key, typename Val>
std::unordered_map<Key, Val> IterableTools::CombineMaps(
    const std::unordered_map<Key, Val>& input_map, const std::unordered_map<Key, Val>& update_map) const
{
    std::unordered_map<Key, Val> new_map = input_map;
    // Iterate over the update map
    // Add keys and values to the input map
    // that exist in the update map, but not in the input map
    for (typename std::unordered_map<Key, Val>::const_iterator it = update_map.begin(); it != update_map.end(); it++)
    {
        if (new_map.count(it->first) == 0)
            new_map[it->first] = it->second;
    }
    return new_map;
}

template <typename T>
std::set<T> IterableTools::VecToSet(const std::vector<T>& input_vec) const
{
    return std::set<T>(input_vec.begin(), input_vec.end());
}

template <typename Key, typename Val>
std::set<Key> IterableTools::GetKeysByValue(const std::map<Key, Val>& input_map,
                                            const Val& match_val) const
{
    std::set<Key> match_keys;
    for (typename std::map<Key, Val>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        if (it->second == match_val)
            match_keys.insert(it->first);
    }
    return match_keys;
}

template <typename Key, typename Val>
std::set<Key> IterableTools::GetKeysByValue(const std::unordered_map<Key, Val>& input_map,
                                            const Val& match_val) const
{
    std::set<Key> match_keys;
    for (typename std::unordered_map<Key, Val>::const_iterator it = input_map.begin();
         it != input_map.end(); ++it)
    {
        if (it->second == match_val)
            match_keys.insert(it->first);
    }
    return match_keys;
}

template <typename T>
bool IterableTools::EqualSets(const std::set<T>& set1, const std::set<T>& set2) const
{
    IterableTools it;

    std::set<T> inter = it.Intersection(set1, set2);
    if (inter.size() == set2.size() && inter.size() == set1.size())
        return true;
    else
        return false;
}

template <typename T>
std::set<T> IterableTools::DeleteFromSet(const std::set<T>& original, const std::set<T>& deletions) const
{
    std::set<T> return_set = original;
    for (auto value : deletions)
    {
        if (return_set.find(value) != return_set.end())
            return_set.erase(value);
    }
    return return_set;
}

template <typename T>
std::set<T> IterableTools::DeleteFromSet(const std::set<T>& original, T deletion) const
{
    std::set<T> return_set = original;
    if (return_set.find(deletion) != return_set.end())
        return_set.erase(deletion);

    return return_set;
}

template <typename Key, typename Val>
std::map<Val, std::set<Key>> IterableTools::ReverseMapSet(const std::map<Key, std::set<Val>>& input_map) const
{
    // Create empty return map
    std::map<Val, std::set<Key>> return_map;

    for (typename std::map<Key, std::set<Val>>::const_iterator it = input_map.begin(); it != input_map.end(); it++)
    {
        for (auto set_value : it->second)
        {
            // if the set value is not in the map already, add the set value as a key to an empty set
            if (!IsKeyInMap(return_map, set_value))
                return_map[set_value] = std::set<Key>();

            return_map[set_value].insert(it->first);
        }
    }

    return return_map;
}

template <typename Key, typename Val>
std::unordered_map<Val, std::set<Key>> IterableTools::ReverseMapSet(const std::unordered_map<Key, std::set<Val>>& input_map) const 
{
    // Create empty return map
    std::unordered_map<Val, std::set<Key>> return_map;

    for (typename std::unordered_map<Key, std::set<Val>>::const_iterator it = input_map.begin(); it != input_map.end(); it++)
    {
        for (auto set_value : it->second)
        {
            // if the set value is not in the map already, add the set value as a key to an empty set
            if (!IsKeyInMap(return_map, set_value))
                return_map[set_value] = std::set<Key>();

            return_map[set_value].insert(it->first);
        }
    }

    return return_map;
}

template <typename T>
size_t IterableTools::IndexOf(const std::vector<T>& input, const T& search_key) const
{
    typename std::vector<T>::const_iterator it = std::lower_bound(input.cbegin(), input.cend(), search_key);
    if (it != input.cend() && (*it == search_key))
    {
        return size_t(std::distance(input.cbegin(), it));
    }
    else
        return std::string::npos;
}

template <typename T>
size_t IterableTools::IndexOf(const std::set<T>& input, const T& search_key) const
{
    typename std::set<T>::const_iterator it = std::lower_bound(input.cbegin(), input.cend(), search_key);
    if (it != input.cend() && (*it == search_key))
    {
        return size_t(std::distance(input.cbegin(), it));
    }
    else
        return std::string::npos;
}

template <typename Key, typename Val>
std::string IterableTools::GetPrintableMapElements_KeyToSet(std::map<Key, std::set<Val>> input_map) const
{
    std::stringstream ss;

    // Sort output by largest set first
    std::vector<Key> keys = SortMapToIterableByValueSize(input_map);
    std::reverse(keys.begin(), keys.end());
    //for (typename std::map<Key, std::set<Val>>::const_iterator it = input_map.begin(); it != input_map.end(); it++)
    for (int i = 0; i < keys.size(); i++)
    {
        ss << " " << keys[i] << ":\t";
        for (auto set_val : input_map[keys[i]])
        {
            ss << set_val << ",";
        }
        // remove last comma if set is greater than one
        if (input_map[keys[i]].size() > 0)
            ss.seekp(-1, std::ios_base::end);

        ss << "\n";
    }

    return ss.str();
}

template <typename Key, typename Val>
std::string IterableTools::GetPrintableMapElements_KeyToValue(const std::map<Key, Val>& input_map) const
{
    std::stringstream ss;
    for (typename std::map<Key, Val>::const_iterator it = input_map.begin(); it != input_map.end(); it++)
    {
        ss << " " << it->first << ":\t";
        ss << it->second;
        ss << "\n";
    }

    return ss.str();
}

template <typename Key>
std::string IterableTools::GetPrintableMapElements_KeyToBool(const std::map<Key, bool>& input_map) const
{
    std::stringstream ss;
    for (typename std::map<Key, bool>::const_iterator it = input_map.begin(); it != input_map.end(); it++)
    {
        ss << " " << it->first << ":\t\t";
        ss << std::boolalpha << it->second;
        ss << "\n";
    }

    return ss.str();
}

template <typename Key, typename Val>
std::string IterableTools::PrintMapWithHeader_KeyToSet(const std::map<Key, std::set<Val>>& input_map, std::vector<std::string> columns, std::string map_name) const
{
    std::stringstream ss;
    ss << GetHeader(columns, map_name);
    ss << GetPrintableMapElements_KeyToSet(input_map);
    ss << GetPrintBar() << "\n\n";
    return ss.str();
}

template <typename Key, typename Val>
std::string IterableTools::PrintMapWithHeader_KeyToValue(const std::map<Key, Val>& input_map, std::vector<std::string> columns, std::string map_name) const
{
    std::stringstream ss;
    ss << GetHeader(columns, map_name);
    ss << GetPrintableMapElements_KeyToValue(input_map);
    ss << GetPrintBar() << "\n\n";
    return ss.str();
}

template <typename Key>
std::string IterableTools::PrintMapWithHeader_KeyToBool(const std::map<Key, bool>& input_map, std::vector<std::string> columns, std::string map_name) const
{
    std::stringstream ss;
    ss << GetHeader(columns, map_name);
    ss << GetPrintableMapElements_KeyToBool(input_map);
    ss << GetPrintBar() << "\n\n";
    return ss.str();
}

template <typename Key, typename Val>
void IterableTools::EmitKeyValuePairWithinMap(YAML::Emitter& e, const Key& input_key,
                                     const Val& input_value) const
{
    e << YAML::Key << input_key;
    e << YAML::Value << input_value;
}

template <typename Key, typename Val>
void IterableTools::EmitKeyValuePair(YAML::Emitter& e, const Key& input_key,
                                     const Val& input_value) const
{
    e << YAML::BeginMap;
    EmitKeyValuePairWithinMap(e, input_key, input_value);
    e << YAML::EndMap;
}

template <typename Key, typename Val>
void IterableTools::EmitSimpleMapWithinMap(YAML::Emitter& e, 
                            const std::map<Key, Val>& input_map,
                            const std::string& map_name) const
{
    e << YAML::Key << map_name;
    e << YAML::Value << input_map;
}

template <typename Key, typename Val>
void IterableTools::EmitSimpleMap(YAML::Emitter& e,
                                  const std::map<Key, Val>& input_map, const std::string& map_name) const
{
    e << YAML::BeginMap;
    EmitSimpleMapWithinMap(e, input_map, map_name);
    e << YAML::EndMap;
}

template <typename Key, typename Val>
void IterableTools::EmitCompoundMapToSetWithinMap(YAML::Emitter& e,
                                         const std::map<Key, std::set<Val>>& input_map, const std::string& map_name) const
{
    e << YAML::Key << map_name;
    e << YAML::Value << YAML::BeginMap;
    typename std::map<Key, std::set<Val>>::const_iterator it;
    for (it = input_map.cbegin(); it != input_map.cend(); ++it)
    {
        std::vector<Val> intermediate(it->second.begin(), it->second.end());
        e << YAML::Key << it->first << YAML::Value << YAML::Flow << intermediate;
    }
    e << YAML::EndMap;
}

template <typename Key, typename Val>
void IterableTools::EmitCompoundMapToSet(YAML::Emitter& e,
                                         const std::map<Key, std::set<Val>>& input_map, const std::string& map_name) const
{
    e << YAML::BeginMap;
    EmitCompoundMapToSetWithinMap(e, input_map, map_name);
    e << YAML::EndMap;
}

template <typename Key, typename Val>
void IterableTools::EmitCompoundMapToVectorWithinMap(YAML::Emitter& e,
                                            const std::map<Key, std::vector<Val>>& input_map, const std::string& map_name) const
{
    e << YAML::Key << map_name;
    e << YAML::Value << YAML::BeginMap;
    typename std::map<Key, std::vector<Val>>::const_iterator it;
    for (it = input_map.cbegin(); it != input_map.cend(); ++it)
    {
        std::vector<Val> intermediate(it->second.begin(), it->second.end());
        e << YAML::Key << it->first << YAML::Value << YAML::Flow << intermediate;
    }
    e << YAML::EndMap;
}

template <typename Key, typename Val>
void IterableTools::EmitCompoundMapToVector(YAML::Emitter& e,
                                            const std::map<Key, std::vector<Val>>& input_map, const std::string& map_name) const
{
    e << YAML::BeginMap;
    EmitCompoundMapToVectorWithinMap(e, input_map, map_name);
    e << YAML::EndMap;
}

template <typename Val>
void IterableTools::EmitSequenceOfVectors(YAML::Emitter& e,
                                          const std::vector<std::vector<Val>>& input_vec) const
{
    e << YAML::BeginSeq;

    typename std::vector<std::vector<Val>>::const_iterator it;
    for (it = input_vec.cbegin(); it != input_vec.cend(); ++it)
    {
        e << YAML::Flow << *it;
    }

    e << YAML::EndSeq;
}

template <typename Key, typename Val>
std::map<Key, std::set<Val>> IterableTools::CombineCompoundMapsToSet(
    const std::map<Key, std::set<Val>>& input_map,
    const std::map<Key, std::set<Val>>& update_map) const
{
    // Initialize the output map with the key/value pairs of
    // the input map.
    std::map<Key, std::set<Val>> out = input_map;
    typename std::map<Key, std::set<Val>>::const_iterator it;

    // Iterate over the update map
    for (it = update_map.begin(); it != update_map.end(); ++it)
    {
        // If the current key is not present in the input map,
        // add the update map key/value pair to the input map.
        if (out.count(it->first) == 0)
            out[it->first] = it->second;
        // Otherwise, insert elements from the update map set into
        // the input map set. This will include any new elements
        // from the update map set that are not present in the
        // input map set.
        else
            out[it->first].insert(it->second.begin(), it->second.end());
    }
    return out;
}

#endif
