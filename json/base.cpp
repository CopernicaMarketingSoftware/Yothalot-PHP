/**
 *  Base.cpp
 *
 *  @author Toon Schoenmakers <toon.schoemakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include "base.h"

#include <json-c/json.h>

/**
 *  Set up namespace
 */
namespace JSON {

/**
 *  Helper method to convert a Json::Value to a string
 *  @param  value
 *  @return string
 */
std::string Base::toString(json_object *value)
{
    switch (json_object_get_type(value)) {
        case json_type_string:  return std::string(json_object_get_string(value), json_object_get_string_len(value));
        case json_type_int:     return std::to_string(toInt(value));
        case json_type_double:  return std::to_string(toDecimal(value));
        default:                return std::string();
    }
}

/**
 *  Helper method to convert a Json::Value to an integer
 *  @param  value
 *  @return integer
 */
int64_t Base::toInt(json_object *value)
{
    switch (json_object_get_type(value)) {
    case json_type_string:  return atoi(json_object_get_string(value));
    case json_type_int:     return json_object_get_int64(value);
    case json_type_double:  return json_object_get_double(value);
    case json_type_boolean: return json_object_get_boolean(value) ? 1 : 0;
    default:                return 0;
    }
}

/**
 *  Helper method to convert a Json::Value to a boolean
 *  @param  value
 *  @return bool
 */
bool Base::toBoolean(json_object *value)
{
    switch (json_object_get_type(value)) {
    case json_type_boolean: return json_object_get_boolean(value);
    default:                return toInt(value) != 0;
    }
}

/**
 *  Helper method to convert a Json::Value to a decimal
 *  @param  value
 *  @return double
 */
double Base::toDecimal(json_object *value)
{
    switch (json_object_get_type(value)) {
    case json_type_string:  return atof(json_object_get_string(value));
    case json_type_int:     return json_object_get_int(value);
    case json_type_double:  return json_object_get_double(value);
    case json_type_boolean: return json_object_get_boolean(value) ? 1.0 : 0.0;
    default:                return 0.0;
    }
}

/**
 *  Create a deep copy of a json value
 *
 *  @param  value
 *  @return json_object
 */
json_object *Base::clone(json_object *value)
{
    // what type of object are we dealing with?
    switch (json_object_get_type(value)) {
        case json_type_null:    return nullptr;
        case json_type_boolean: return json_object_new_boolean(json_object_get_boolean(value));
        case json_type_double:  return json_object_new_double(json_object_get_double(value));
        case json_type_int:     return json_object_new_int(json_object_get_int(value));
        case json_type_string:  return json_object_new_string_len(json_object_get_string(value), json_object_get_string_len(value));
        case json_type_object:  {
            // create a new value
            auto *result = json_object_new_object();

            // loop through the object
            json_object_object_foreach(value, key, val)
            {
                // add a new value to the object
                json_object_object_add(result, key, Base::clone(val));
            }

            // return the filled object
            return result;
        }
        case json_type_array: {
            // create a new value
            auto *result = json_object_new_array();

            // loop through the array
            for (int i = 0; i < json_object_array_length(value); ++i)
            {
                // add a new value to the array
                json_object_array_put_idx(result, i, Base::clone(json_object_array_get_idx(value, i)));
            }

            // return the filled array
            return result;
        }
    }

    // this is not possible, because all different options were already caught
    // but the compiler likes us to be sure, so we just return a nullptr here
    return nullptr;
}

/**
 *  Custom output stream overloader, so that JSON objects can be used as stream arguments
 *  @param  stream
 *  @param  json
 *  @return std::ostream
 */
std::ostream& operator<<(std::ostream& stream, const JSON::Base &json)
{
    stream << json.toString();
    return stream;
}

/**
 *  End of namespace
 */
}