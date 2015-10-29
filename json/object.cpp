/**
 *  Object.cpp
 *
 *  Implementation for the json object
 *
 *  @author Toon Schoenmakers <toon.schoemakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include "object.h"

/**
 *  Set up namespace
 */
namespace JSON {

/**
 *  Size of the object (number of properties)
 *  @return integer
 */
int Object::size() const
{
    return json_object_object_length(_json);
}

/**
 *  Check if a certain member is set
 *  @param  name        Name of the member
 *  @return bool
 */
bool Object::contains(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    return json_object_object_get_ex(_json, name, &value);
}

/**
 *  Retrieve all member names
 *  This effectively returns a vector of strings
 *  @return vector
 */
std::vector<const char*> Object::members() const
{
    // result variable
    std::vector<const char*> result;
    result.reserve(size());

    // loop through the object
    json_object_object_foreach(_json, key, val)
    {
        // append
        result.emplace_back(key);

        // prevent compiler warnings
        (void)val;
    }

    // done
    return result;
}

/**
 *  Retrieve the type of the element at a certain position
 *
 *  @param  name    the name of the element to retrieve the type for
 *  @return Type
 */
Type Object::type(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return Type::Undefined;

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
bool Object::isString(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return false;

    // check the type
    return json_object_is_type(value, json_type_string);
}

/**
 *  Check if the element at a certain position is an int
 *  @param  index
 *  @return bool
 */
bool Object::isInteger(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return false;

    // check the type
    return json_object_is_type(value, json_type_int);
}

/**
 *  Check if the element at a certain position is a boolean
 *  @param  index
 *  @return bool
 */
bool Object::isBoolean(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return false;

    // check the type
    return json_object_is_type(value, json_type_boolean);
}

/**
 *  Check if the element at a certain position is a floating point number
 *  @param  index
 *  @return bool
 */
bool Object::isDecimal(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return false;

    // check the type
    return json_object_is_type(value, json_type_double);
}

/**
 *  Check if the element at a certain position is an object
 *  @param  index
 *  @return bool
 */
bool Object::isObject(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return false;

    // check the type
    return json_object_is_type(value, json_type_object);
}

/**
 *  Check if the element at a certain position is an array
 *  @param  index
 *  @return bool
 */
bool Object::isArray(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return false;

    // check the type
    return json_object_is_type(value, json_type_array);
}

/**
 *  Check if the element at a certain position is null
 *  @param  name
 *  @return bool
 */
bool Object::isNull(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return false;

    // check the type
    return json_object_is_type(value, json_type_null);
}

/**
 *  Get a string member
 *
 *  The returned pointer is valid as long as this
 *  object stays in scope.
 *
 *  @return const char
 */
const char *Object::c_str(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return "";

    // in case our type isn't string we return an empty string
    if (json_object_get_type(value) != json_type_string) return "";

    // cast to a char pointer
    return json_object_get_string(value);
}

/**
 *  Get the string length at a certain key
 *  @param  index
 *  @return string
 */
size_t Object::strlen(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return 0;

    // in case our type isn't string we return an empty string
    if (json_object_get_type(value) != json_type_string) return 0;

    // cast to a char pointer
    return json_object_get_string_len(value);
}

/**
 *  Get a integer member
 *  @param  name        Name of the member
 *  @return int
 */
int64_t Object::integer(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return 0;

    // cast to int
    return toInt(value);
}

/**
 *  Get a decimal member
 *  @param  name        Name of the string
 *  @return integer
 */
double Object::decimal(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return 0.0;

    // cast to decimal
    return toDecimal(value);
}

/**
 *  Get a boolean member
 *  @param  name        Name of the member
 *  @return bool
 */
bool Object::boolean(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return false;

    // cast to boolean
    return toBoolean(value);
}

/**
 *  Set an integer member
 *  @param  name        Name of the member
 *  @param  value       Value to set
 */
void Object::set(const char *name, int value)
{
    // set the value
    json_object_object_add(_json, name, json_object_new_int(value));
}

/**
 *  Set a boolean member
 *
 *  @param  name        Name of the member
 *  @param  value       Value to set
 */
void Object::set(const char *name, bool value)
{
    // set the value
    json_object_object_add(_json, name, json_object_new_boolean(value));
}

/**
 *  Set an integer member
 *  @param  name        Name of the member
 *  @param  value       Value to set
 */
void Object::set(const char *name, int64_t value)
{
    // set the value
    json_object_object_add(_json, name, json_object_new_int64(value));
}

/**
 *  Set a string member
 *  @param  name        Name of the member
 *  @param  value       Value to set
 */
void Object::set(const char *name, const char *value)
{
    // set the value
    json_object_object_add(_json, name, json_object_new_string(value));
}

/**
 *  Set a decimal member
 *  @param  name        Name of the member
 *  @param  value       Value to set
 */
void Object::set(const char *name, double value)
{
    // set the value
    json_object_object_add(_json, name, json_object_new_double(value));
}

/**
 *  Set a member to be an array
 *  @param  name        Name of the member
 *  @param  value       Value to set
 */
void Object::set(const char *name, const Array &value)
{
    // set the value
    json_object_object_add(_json, name, json_object_get(value._json));
}

/**
 *  Set a member to be an object
 *  @param  name        Name of the member
 *  @param  value       Value to set
 */
void Object::set(const char *name, const Object &value)
{
    // set the value
    json_object_object_add(_json, name, json_object_get(value._json));
}

/**
 *  Set a member to be null
 *  @param  name       Name of the member
 *  @param  value      Value to set
 */
void Object::set(const char *name, const std::nullptr_t null)
{
    // set the value
    json_object_object_add(_json, name, null);
}

/**
 *  Remove a member
 *  @param  name        Member to remove
 */
void Object::remove(const char *name)
{
    // remove the member
    json_object_object_del(_json, name);
}

/**
 *  Get an array member
 *  @param  name        Name of the member
 *  @return Array
 */
Array Object::array(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return Array();

    // wrap in array object
    return Array(value);
}

/**
 *  Get an object member
 *  @param  name        Name of the member
 *  @return Object
 */
Object Object::object(const char *name) const
{
    // the value
    json_object *value = nullptr;

    // check if the key exists
    if (!json_object_object_get_ex(_json, name, &value)) return Object();

    // wrap in Object object
    return Object(value);
}

/**
 *  Create a deep copy of this object
 *
 *  Note that this can be quite a heavy operation, depending on the size
 *  of the object that is being copied. If no modification is required,
 *  it will be better (and faster) to not make a copy.
 *
 *  @return Object
 */
Object Object::clone() const
{
    // create a copy of the value
    auto *copy = Base::clone(_json);

    // create a new object around it
    Object result(copy);

    // decrement the refcount
    json_object_put(copy);

    // and return the result
    return result;
}

/**
 *  Turn into a Php::Value
 *  @return Php::Value
 */
Php::Value Object::phpValue() const
{
    // create our output value
    Php::Value output;

    // loop over all the keys
    for (auto key : members())
    {
        // switch through all the types and add them to our php value
        switch (type(key))
        {
        case Type::Null:    output.set(key, nullptr);                  break;
        case Type::Boolean: output.set(key, boolean(key));             break;
        case Type::Decimal: output.set(key, decimal(key));             break;
        case Type::Integer: output.set(key, integer(key));             break;
        case Type::String:  output.set(key, c_str(key));               break;
        case Type::Array:   output.set(key, array(key).phpValue());    break;
        case Type::Object:  output.set(key, object(key).phpValue());   break;
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