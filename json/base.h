/**
 *  Base.h
 *
 *  Base class for Json objects and Json arrays
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
#include <json-c/json.h>
#include <string>

/**
 *  Set up namespace
 */
namespace JSON {

/**
 *  Class definition
 */
class Base
{
protected:
    /**
     *  The actual JSON object
     *  @var    json_object
     */
    json_object *_json = nullptr;

    /**
     *  Helper method to convert a Json::Value to a string
     *  @param  value
     *  @return string
     */
    static std::string toString(json_object *value);

    /**
     *  Helper method to convert a Json::Value to an integer
     *  @param  value
     *  @return integer
     */
    static int64_t toInt(json_object *value);

    /**
     *  Helper method to convert a Json::Value to a boolean
     *  @param  value
     *  @return bool
     */
    static bool toBoolean(json_object *value);

    /**
     *  Helper method to convert a Json::Value to a decimal
     *  @param  value
     *  @return double
     */
    static double toDecimal(json_object *value);

    /**
     *  Create a deep copy of a json value
     *
     *  @param  value
     *  @return json_object
     */
    static json_object *clone(json_object *value);


public:
    /**
     *  Constructor
     */
    Base() {}

    /**
     *  Construct to make a certain type
     *  @param  int
     */
    Base(json_type type)
    {
        switch (type) {
        case json_type_boolean: _json = json_object_new_boolean(false); break;
        case json_type_double:  _json = json_object_new_double(0.0); break;
        case json_type_int:     _json = json_object_new_int(0); break;
        case json_type_object:  _json = json_object_new_object(); break;
        case json_type_array:   _json = json_object_new_array(); break;
        case json_type_string:  _json = json_object_new_string_len("", 0); break;
        default:                _json = nullptr; break;
        }
    }

    /**
     *  Constructor
     *
     *  @param  message     Message to parse
     */
    Base(const std::string &message) : Base(message.c_str(), message.size()) {}

    /**
     *  Constructor
     *
     *  @param  message     Message to parse
     *  @param  size        Size of the message
     */
    Base(const char *message, size_t size = -1)
    {
        // create a tokener
        auto *tokener = json_tokener_new();

        // check if the tokener was created
        if (tokener == nullptr) return;

        // parse the message
        _json = json_tokener_parse_ex(tokener, message, size);

        // check for success
        if (_json && json_tokener_get_error(tokener) != json_tokener_success)
        {
            // there is a failure, destruct the json object
            json_object_put(_json);

            // set back to null
            _json = nullptr;
        }

        // destruct the tokener
        json_tokener_free(tokener);
    }

    /**
     *  Copy constructor
     *
     *  A copy of a JSON object refers to the same underlying object
     */
    Base(const Base &json) : _json(json_object_get(json._json)) {}

    /**
     *  Move constructor
     *  @param  json        The other object
     */
    Base(Base &&json) : _json(json._json)
    {
        // other object can be invalidated
        json._json = nullptr;
    }

    /**
     *  Destructor
     */
    virtual ~Base()
    {
        // decrement refcount
        if (_json) json_object_put(_json);
    }

    /**
     *  Retrieve the internal json_object pointer
     *  @return json_object*
     */
    json_object *internal()
    {
        return _json;
    }

    /**
     *  Assignment operator
     *  @param  json        Object to copy
     *  @return Json
     */
    Base &operator=(const Base &json)
    {
        // skip self assignment
        if (this == &json) return *this;

        // decrement previous object
        if (_json) json_object_put(_json);

        // a copy refers to the same object
        _json = json_object_get(json._json);

        // done
        return *this;
    }

    /**
     *  Convert to a const char *
     *  @return const char *
     */
    operator const char * () const
    {
        return json_object_to_json_string(_json);
    }

    /**
     *  Convert the object to a string
     *  @return string
     */
    std::string toString() const
    {
        // convert to a string
        return json_object_to_json_string(_json);
    }

    /**
     *  Cast to a string
     *  @return string
     */
    operator std::string () const
    {
        // call the toString method
        return toString();
    }

    /**
     *  Objects and arrays may access private stuff
     */
    friend class Object;
    friend class Array;
};

/**
 *  Custom output stream overloader, so that JSON objects can be used as stream arguments
 *  @param  stream
 *  @param  json
 *  @return std::ostream
 */
extern std::ostream& operator<<(std::ostream& stream, const JSON::Base &json);

/**
 *  End of namespace
 */
}