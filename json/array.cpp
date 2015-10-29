/**
 *  Array.cpp
 *
 *  Implementation for the JSON array class
 *
 *  @author Toon Schoenmakers <toon.schoemakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include "array.h"
#include "object.h"
#include <cstring>

/**
 *  Set up namespace
 */
namespace JSON {

/**
 *  Number of elements
 *  @return int
 */
int Array::size() const
{
    return json_object_array_length(_json);
}

/**
 *  Retrieve the type of the element at a certain position
 *
 *  @param  name    the name of the element to retrieve the type for
 *  @return Type
 */
Type Array::type(int index) const
{
    // the value
    json_object *value = json_object_array_get_idx(_json, index);

    // check if the key exists
    if (!value) return Type::Undefined;

    // map internal json-c value to external value
    switch (json_object_get_type(value))
    {
        case json_type_null:    return Type::Null;
        case json_type_boolean: return Type::Boolean;
        case json_type_double:  return Type::Decimal;
        case json_type_int:     return Type::Integer;
        case json_type_object:  return Type::Object;
        case json_type_array:   return Type::Array;
        case json_type_string:  return Type::String;
    }

    // the compiler knows that we have got all
    // options covered, but it still cribs about
    // not having a return statement here
    return Type::Undefined;
}

/**
 *  Check if the element at a certain position is a string
 *  @param  index
 *  @return bool
 */
bool Array::isString(int index) const
{
    // check type
    return json_object_is_type(json_object_array_get_idx(_json, index), json_type_string);
}

/**
 *  Check if the element at a certain position is an int
 *  @param  index
 *  @return bool
 */
bool Array::isInteger(int index) const
{
    // check type
    return json_object_is_type(json_object_array_get_idx(_json, index), json_type_int);
}

/**
 *  Check if the element at a certain position is a boolean
 *  @param  index
 *  @return bool
 */
bool Array::isBoolean(int index) const
{
    // check type
    return json_object_is_type(json_object_array_get_idx(_json, index), json_type_boolean);
}

/**
 *  Check if the element at a certain position is a floating point number
 *  @param  index
 *  @return bool
 */
bool Array::isDecimal(int index) const
{
    // check type
    return json_object_is_type(json_object_array_get_idx(_json, index), json_type_double);
}

/**
 *  Check if the element at a certain position is an object
 *  @param  index
 *  @return bool
 */
bool Array::isObject(int index) const
{
    // check type
    return json_object_is_type(json_object_array_get_idx(_json, index), json_type_object);
}

/**
 *  Check if the element at a certain position is an array
 *  @param  index
 *  @return bool
 */
bool Array::isArray(int index) const
{
    // check type
    return json_object_is_type(json_object_array_get_idx(_json, index), json_type_array);
}

/**
 *  Check if the element at a certain position is null
 *  @param  index
 *  @return bool
 */
bool Array::isNull(int index) const
{
    // check type
    return json_object_is_type(json_object_array_get_idx(_json, index), json_type_null);
}

/**
 *  Get a string value at a certain index
 *  @param  index
 *  @return string
 */
const char* Array::c_str(int index) const
{
    // get the value at this location
    json_object *value = json_object_array_get_idx(_json, index);

    // in case our type isn't string we return an empty string
    if (json_object_get_type(value) != json_type_string) return "";

    // cast to a char pointer
    return json_object_get_string(value);
}

/**
 *  Get the string length at a certain index
 *  @param  index
 *  @return string
 */
size_t Array::strlen(int index) const
{
    // get the value at this location
    json_object *value = json_object_array_get_idx(_json, index);

    // in case our type isn't string we return an empty string
    if (json_object_get_type(value) != json_type_string) return 0;

    // get the string length
    return json_object_get_string_len(value);
}

/**
 *  Get a integer value at a certain index
 *  @param  index
 *  @return int64_t
 */
int64_t Array::integer(int index) const
{
    // get value
    return toInt(json_object_array_get_idx(_json, index));
}

/**
 *  Get a boolean value at a certain index
 *  @param  index
 *  @return bool
 */
bool Array::boolean(int index) const
{
    // get value
    return toBoolean(json_object_array_get_idx(_json, index));
}

/**
 *  Get a decimal value at a certain index
 *  @param  index
 *  @return double
 */
double Array::decimal(int index) const
{
    // get value
    return toDecimal(json_object_array_get_idx(_json, index));
}

/**
 *  Check if a certain value appears in the array
 *  @param  value
 *  @return bool
 */
bool Array::contains(const std::string &value) const
{
    // find the size
    int s = size();

    // loop through the array
    for (int i=0; i<s; i++)
    {
        // check
        if (value == c_str(i)) return true;
    }

    // not found
    return false;
}

/**
 *  Check if a certain value appears in the array
 *  @param  value
 *  @return bool
 */
bool Array::contains(const char *value) const
{
    // find the size
    int s = size();

    // loop through the array
    for (int i=0; i<s; i++)
    {
        // check
        if (strcmp(value, c_str(i)) == 0) return true;
    }

    // not found
    return false;
}

/**
 *  Check if a certain value appears in the array
 *  @param  value
 *  @return bool
 */
bool Array::contains(int value) const
{
    // find the size
    int s = size();

    // loop through the array
    for (int i=0; i<s; i++)
    {
        // check
        if (value == integer(i)) return true;
    }

    // not found
    return false;
}

/**
 *  Check if a certain value appears in the array
 *  @param  value
 *  @return bool
 */
bool Array::contains(bool value) const
{
    // find the size
    int s = size();

    // loop through the array
    for (int i=0; i<s; i++)
    {
        // check
        if (value == boolean(i)) return true;
    }

    // not found
    return false;
}

/**
 *  Check if a certain value appears in the array
 *  @param  value
 *  @return bool
 */
bool Array::contains(double value) const
{
    // find the size
    int s = size();

    // loop through the array
    for (int i=0; i<s; i++)
    {
        // check
        if (value == decimal(i)) return true;
    }

    // not found
    return false;
}

/**
 *  Append a value to the array
 *  @param  value
 */
void Array::append(const std::string &value)
{
    // add property
    json_object_array_add(_json, json_object_new_string_len(value.c_str(), value.size()));
}

/**
 *  Append a value to the array
 *  @param  value
 */
void Array::append(const char *value)
{
    // add property
    json_object_array_add(_json, json_object_new_string(value));
}

/**
 *  Append a value to the array
 *
 *  @param  value   The value to add
 *  @param  size    The number of characters in the value
 */
void Array::append(const char *value, size_t size)
{
    // add property
    json_object_array_add(_json, json_object_new_string_len(value, size));
}

/**
 *  Append a value to the array
 *  @param  value
 */
void Array::append(int value)
{
    // add property
    json_object_array_add(_json, json_object_new_int(value));
}

/**
 *  Append a value to the array
 *  @param  value
 */
void Array::append(int64_t value)
{
    // add property
    json_object_array_add(_json, json_object_new_int64(value));
}

/**
 *  Append a value to the array
 *  @param  value
 */
void Array::append(bool value)
{
    // add property
    json_object_array_add(_json, json_object_new_boolean(value));
}

/**
 *  Append a value to the array
 *  @param  value
 */
void Array::append(double value)
{
    // add property
    json_object_array_add(_json, json_object_new_double(value));
}

/**
 *  Append a value to the array
 *  @param  value
 */
void Array::append(const Array &value)
{
    // add property
    json_object_array_add(_json, json_object_get(value._json));
}

/**
 *  Append a value to the array
 *  @param  value
 */
void Array::append(const Object &value)
{
    // add property
    json_object_array_add(_json, json_object_get(value._json));
}

/**
 *  Append a value to the array
 *  @param value
 */
void Array::append(const std::nullptr_t value)
{
    // add property
    json_object_array_add(_json, value);
}

/**
 *  Get an object value at a certain index
 *  @param  index
 *  @return Object
 */
Array Array::array(int index) const
{
    // get the value
    return Array(json_object_array_get_idx(_json, index));
}

/**
 *  Get an object value at a certain index
 *  @param  index
 *  @return Object
 */
Object Array::object(int index) const
{
    // get the value
    return Object(json_object_array_get_idx(_json, index));
}

/**
 *  Create a deep copy of this array
 *
 *  Note that this can be quite a heavy operation, depending on the size
 *  of the array that is being copied. If no modification is required,
 *  it will be better (and faster) to not make a copy.
 *
 *  @return Array
 */
Array Array::clone() const
{
    // create a copy of the value
    auto *copy = Base::clone(_json);

    // create a new array around it
    Array result(copy);

    // decrement the refcount
    json_object_put(copy);

    // and return the result
    return result;
}

/**
 *  Turn into a Php::Value
 *  @return Php::Value
 */
Php::Value Array::phpValue() const
{
    // create an output value
    Php::Value output;

    // loop over the entire array
    for (int i = 0; i < size(); ++i)
    {
        // switch through all the types and add them to the php value
        switch (type(i))
        {
        case JSON::Type::Null:    output.set(i, nullptr);              break;
        case JSON::Type::Boolean: output.set(i, boolean(i));           break;
        case JSON::Type::Decimal: output.set(i, decimal(i));           break;
        case JSON::Type::Integer: output.set(i, integer(i));           break;
        case JSON::Type::String:  output.set(i, c_str(i));             break;
        case JSON::Type::Array:   output.set(i, array(i).phpValue());  break;
        case JSON::Type::Object:  output.set(i, object(i).phpValue()); break;
        default:                                                       break;
        }
    }

    // return our output
    return output;
}

/**
 *  End of namespace
 */
}