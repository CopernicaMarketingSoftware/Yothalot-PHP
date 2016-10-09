/**
 *  Object.h
 *
 *  Class for parsing a JSON string into string members
 *
 *  @author Toon Schoenmakers <toon.schoemakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "base.h"
#include "type.h"
#include "array.h"
#include <map>
#include <vector>
#include <phpcpp.h>

/**
 *  Set up namespace
 */
namespace JSON {

/**
 *  Class definition
 */
class Object : public Base
{
public:
    /**
     *  Constructor
     */
    Object() : Base(json_type_object) {}

    /**
     *  Constructor
     *  @param  message     The string to parse
     */
    Object(const std::string &message) : Base(message)
    {
        // must be an object
        if (json_object_get_type(_json) == json_type_object) return;

        // decrement refcount
        if (_json) json_object_put(_json);

        // create new object
        _json = json_object_new_object();
    }

    /**
     *  Constructor
     *  @param  message     The string to parse
     *  @param  size        Size of the buffer
     */
    Object(const char *message, size_t size = -1) : Base(message, size)
    {
        // must be an object
        if (json_object_get_type(_json) == json_type_object) return;

        // decrement refcount
        if (_json) json_object_put(_json);

        // create new object
        _json = json_object_new_object();
    }

    /**
     *  Wrap around existing internal json object
     *  @param  value       Underlying libjson object
     */
    Object(json_object *object)
    {
        // is this an object of type object? if so we increment refcount
        if (json_object_get_type(object) == json_type_object) _json = json_object_get(object);

        // create a new array
        else _json = json_object_new_object();
    }

    /**
     *  Constructor from an std::map
     *  @param  map
     */
    template<typename Key, typename Value>
    Object(const std::map<Key, Value> &map) : Object()
    {
        // loop over all the items and set all of them
        for (auto iter : map) set(iter.first, iter.second);
    }

    /**
     *  Destructor
     */
    virtual ~Object() = default;

    /**
     *  Size of the object (number of properties)
     *  @return integer
     */
    int size() const;

    /**
     *  Is a certain option set?
     *  @param  name        Name of the key
     *  @return boolean
     */
    bool contains(const std::string &name) const { return contains(name.c_str()); }

    /**
     *  Is a certain option set?
     *  @param  name        Name of the key
     *  @return boolean
     */
    bool contains(const char *name) const;

    /**
     *  Retrieve all member names
     *  This effectively returns a vector of strings
     *  @return vector
     */
    std::vector<const char*> members() const;

    /**
     *  Retrieve the type of the element at a certain position
     *
     *  @param  name    the name of the element to retrieve the type for
     *  @return Type
     */
    Type type(const std::string &name) const { return type(name.c_str()); }
    Type type(const char *name) const;

    /**
     *  Check if the element at a certain position is a string
     *  @param  name
     *  @return bool
     */
    bool isString(const std::string &name) const { return isString(name.c_str()); }
    bool isString(const char *name) const;

    /**
     *  Check if the element at a certain position is an int
     *  @param  name
     *  @return bool
     */
    bool isInteger(const std::string &name) const { return isInteger(name.c_str()); }
    bool isInteger(const char *name) const;

    /**
     *  Check if the element at a certain position is a boolean
     *  @param  name
     *  @return bool
     */
    bool isBoolean(const std::string &name) const { return isBoolean(name.c_str()); }
    bool isBoolean(const char *name) const;

    /**
     *  Check if the element at a certain position is a floating point number
     *  @param  name
     *  @return bool
     */
    bool isDecimal(const std::string &name) const { return isDecimal(name.c_str()); }
    bool isDecimal(const char *name) const;

    /**
     *  Check if the element at a certain position is an object
     *  @param  name
     *  @return bool
     */
    bool isObject(const std::string &name) const { return isObject(name.c_str()); }
    bool isObject(const char *name) const;

    /**
     *  Check if the element at a certain position is an array
     *  @param  name
     *  @return bool
     */
    bool isArray(const std::string &name) const { return isArray(name.c_str()); }
    bool isArray(const char *name) const;

    /**
     *  Check if the element at a certain position is null
     *  @param  name
     *  @return bool
     */
    bool isNull(const std::string &name) const { return isNull(name.c_str()); }
    bool isNull(const char *name) const;

    /**
     *  Get a string member
     *
     *  The returned pointer is valid as long as this
     *  object stays in scope.
     *
     *  @return const char
     */
    const char *c_str(const char *name) const;
    size_t strlen(const char *name) const;

    /**
     *  Get a integer member
     *  @param  name        Name of the string
     *  @return integer
     */
    int64_t integer(const std::string &name) const { return integer(name.c_str()); }
    int64_t integer(const char *name) const;

    /**
     *  Get a decimal member
     *  @param  name        Name of the string
     *  @return integer
     */
    double decimal(const std::string &name) const { return decimal(name.c_str()); }
    double decimal(const char *name) const;

    /**
     *  Get a boolean member
     *  @param  name        Name of the property
     *  @return bool
     */
    bool boolean(const std::string &name) const { return boolean(name.c_str()); }
    bool boolean(const char *name) const;

    /**
     *  Set an integer member
     *  @param  name        Name of the member
     *  @param  value       Value to set
     */
    void set(const std::string &name, int value) { set(name.c_str(), value); }
    void set(const char *name, int value);
    void set(const std::string &name, int64_t value) { set(name.c_str(), value); }
    void set(const char *name, int64_t value);

    /**
     *  Set a boolean member
     *
     *  @param  name        Name of the member
     *  @param  value       Value to set
     */
    void set(const std::string &name, bool value) { set(name.c_str(), value); }
    void set(const char *name, bool value);

    /**
     *  Set a string member
     *  @param  name        Name of the member
     *  @param  value       Value to set
     */
    void set(const std::string &name, const std::string &value) { set(name.c_str(), value.c_str()); }
    void set(const char *name, const std::string &value) { set(name, value.c_str()); }
    void set(const std::string &name, const char *value) { set(name.c_str(), value); }
    void set(const char *name, const char *value);

    /**
     *  Set a decimal member
     *  @param  name        Name of the member
     *  @param  value       Value to set
     */
    void set(const std::string &name, double value) { set(name.c_str(), value); }
    void set(const char *name, double value);

    /**
     *  Set a member to be an array
     *  @param  name        Name of the member
     *  @param  value       Value to set
     */
    void set(const std::string &name, const Array &value) { set(name.c_str(), value); }
    void set(const char *name, const Array &value);

    /**
     *  Set a member to be an object
     *  @param  name        Name of the member
     *  @param  value       Value to set
     */
    void set(const std::string &name, const Object &value) { set(name.c_str(), value); }
    void set(const char *name, const Object &value);

    /**
     *  Set a member to be null
     *  @param  name       Name of the member
     *  @param  value      Value to set
     */
    void set(const std::string &name, const std::nullptr_t null) { set(name.c_str(), null); }
    void set(const char *name, const std::nullptr_t null);

    /**
     *  Remove a member
     *  @param  name        Member to remove
     */
    void remove(const std::string &name) { remove(name.c_str()); }
    void remove(const char *name);

    /**
     *  Get an array member
     *  @param  name        Name of the member
     *  @return JsonArray
     */
    Array array(const std::string &name) const { return array(name.c_str()); }
    Array array(const char *name) const;

    /**
     *  Get an object member
     *  @param  name        Name of the member
     *  @return JsonObject
     */
    Object object(const std::string &name) const { return object(name.c_str()); }
    Object object(const char *name) const;

    /**
     *  Create a deep copy of this object
     *
     *  Note that this can be quite a heavy operation, depending on the size
     *  of the object that is being copied. If no modification is required,
     *  it will be better (and faster) to not make a copy.
     *
     *  @return Object
     */
    Object clone() const;

    /**
     *  Turn into a Php::Value
     *  @return Php::Value
     */
    Php::Value phpValue() const;
};

/**
 *  End of namespace
 */
}
