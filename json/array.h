/**
 *  Array.h
 *
 *  Class for accessing arrays stored in JSON
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
#include <initializer_list>
#include <phpcpp.h>

/**
 *  Set up namespace
 */
namespace JSON {

/**
 *  Forward declarations
 */
class Object;

/**
 *  Class definition
 */
class Array : public Base
{
public:
    /**
     *  Constructor
     */
    Array() : Base(json_type_array)
    {
    }

    /**
     *  Constructor
     *  @param  message     The string to parse
     */
    Array(const std::string &message) : Base(message)
    {
        // must be an array
        if (json_object_get_type(_json) == json_type_array) return;

        // decrement refcount
        if (_json) json_object_put(_json);

        // create new array
        _json = json_object_new_array();
    }

    /**
     *  Constructor
     *  @param  message     The string to parse
     *  @param  size        Size of the buffer
     */
    Array(const char *message, size_t size = -1) : Base(message, size)
    {
        // must be an array
        if (json_object_get_type(_json) == json_type_array) return;

        // decrement refcount
        if (_json) json_object_put(_json);

        // create new array
        _json = json_object_new_array();
    }

    /**
     *  Wrap around existing internal json object
     *  @param  value       Underlying libjson object
     */
    Array(json_object *object)
    {
        // is this an object of type array? if so we increment refcount
        if (json_object_get_type(object) == json_type_array) _json = json_object_get(object);

        // create a new array
        else _json = json_object_new_array();
    }

    /**
     *  Construct a new array with all the items from list in it
     *  @param  list       List of items used for initial construction
     */
    template<typename T>
    Array(const std::initializer_list<T> &list) : Array(list.begin(), list.end()) {}

    /**
     *  Construct a new array from any kind of iterator, this will allow you to
     *  construct one directly from vectors, lists, etc.
     *  @param  begin    The begin iterator
     *  @param  end      The end iterator
     */
    template<typename Iterator>
    Array(const Iterator begin, const Iterator end) : Array()
    {
        for (Iterator iter = begin; iter != end; ++iter) append(*iter);
    }

    /**
     *  Destructor
     */
    virtual ~Array()
    {
    }

    /**
     *  Number of elements
     *  @return int
     */
    int size() const;

    /**
     *  Retrieve the type of the element at a certain position
     *
     *  @param  name    the name of the element to retrieve the type for
     *  @return Type
     */
    Type type(int index) const;

    /**
     *  Check if the element at a certain position is a string
     *  @param  index
     *  @return bool
     */
    bool isString(int index) const;

    /**
     *  Check if the element at a certain position is an int
     *  @param  index
     *  @return bool
     */
    bool isInteger(int index) const;

    /**
     *  Check if the element at a certain position is a boolean
     *  @param  index
     *  @return bool
     */
    bool isBoolean(int index) const;

    /**
     *  Check if the element at a certain position is a floating point number
     *  @param  index
     *  @return bool
     */
    bool isDecimal(int index) const;

    /**
     *  Check if the element at a certain position is an object
     *  @param  index
     *  @return bool
     */
    bool isObject(int index) const;

    /**
     *  Check if the element at a certain position is an array
     *  @param  index
     *  @return bool
     */
    bool isArray(int index) const;

    /**
     *  Check if the element at a certain position is null
     *  @param  index
     *  @return bool
     */
    bool isNull(int index) const;

    /**
     *  Get a string value at a certain index
     *  @param  index
     *  @return string
     */
    const char *c_str(int index) const;
    size_t strlen(int index) const;

    /**
     *  Get a integer value at a certain index
     *  @param  index
     *  @return int64_t
     */
    int64_t integer(int index) const;

    /**
     *  Get a boolean value at a certain index
     *  @param  index
     *  @return bool
     */
    bool boolean(int index) const;

    /**
     *  Get a decimal value at a certain index
     *  @param  index
     *  @return double
     */
    double decimal(int index) const;

    /**
     *  Check if a certain value appears in the array
     *  @param  value
     *  @return bool
     */
    bool contains(const std::string &value) const;

    /**
     *  Check if a certain value appears in the array
     *  @param  value
     *  @return bool
     */
    bool contains(const char *value) const;

    /**
     *  Check if a certain value appears in the array
     *  @param  value
     *  @return bool
     */
    bool contains(int value) const;

    /**
     *  Check if a certain value appears in the array
     *  @param  value
     *  @return bool
     */
    bool contains(bool value) const;

    /**
     *  Check if a certain value appears in the array
     *  @param  value
     *  @return bool
     */
    bool contains(double value) const;

    /**
     *  Append a value to the array
     *  @param  value
     */
    void append(const std::string &value);

    /**
     *  Append a value to the array
     *  @param  value
     */
    void append(const char *value);

    /**
     *  Append a value to the array
     *
     *  @param  value   The value to add
     *  @param  size    The number of characters in the value
     */
    void append(const char *value, size_t size);

    /**
     *  Append a value to the array
     *  @param  value
     */
    void append(int value);

    /**
     *  Append a value to the array
     *  @param  value
     */
    void append(int64_t value);

    /**
     *  Append a value to the array
     *  @param  value
     */
    void append(bool value);

    /**
     *  Append a value to the array
     *  @param  value
     */
    void append(double value);

    /**
     *  Append a value to the array
     *  @param  value
     */
    void append(const Array &value);

    /**
     *  Append a value to the array
     *  @param  value
     */
    void append(const Object &value);

    /**
     *  Append a value to the array
     *  @param value
     */
    void append(const std::nullptr_t value);

    /**
     *  Get an object value at a certain index
     *  @param  index
     *  @return JSONObject
     */
    Object object(int index) const;

    /**
     *  Get an array value at a certain index
     *  @param  index
     *  @return Array
     */
    Array array(int index) const;

    /**
     *  Create a deep copy of this array
     *
     *  Note that this can be quite a heavy operation, depending on the size
     *  of the array that is being copied. If no modification is required,
     *  it will be better (and faster) to not make a copy.
     *
     *  @return Array
     */
    Array clone() const;

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