/**
 *  Tuple.h
 *
 *  A coupld of tuple classes to simplify converting variables
 *
 *  @author    Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 - 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <phpcpp.h>
#include <yothalot.h>
#include "json/array.h"

/**
 *  Begin of namespace
 */
namespace Tuple {

/**
 *  Class to turn a php value into a yothalot tuple
 */
class Yothalot : public ::Yothalot::Tuple
{
public:
    /**
     *  Constructor
     *  @param  value
     */
    Yothalot(const Php::Value &value)
    {
        // if we're numeric or a string we just return a tuple with a single value
        if (value.isNumeric()) add(value.numericValue());
        if (value.isString())  add(value.stringValue());
        
        // we only support arrays and objects as the other type
        if (!value.isArray() && !value.isObject())  return;
        
        // we start looping over it
        for (auto iter : value)
        {
            // we're going to add numeric values as numbers, everything else as strings
            if (iter.second.isNumeric())   add(iter.second.numericValue());
            else if (iter.second.isNull()) add(nullptr);
            else                           add(iter.second.stringValue());
        }
    }
    
    /**
     *  Destructor
     */
    virtual ~Yothalot() = default;
};

/**
 *  Class to convert a yothalot tuple to a php wrapped value
 */
class Php : public ::Php::Value
{
public:
    /**
     *  Constructor
     *  @param  input
     */
    Php(const Yothalot::Tuple &input)
    {
        // if we have a single field we're not building an array
        if (input.fields() == 1)
        {
            // we're going to construct a scalar
            if      (input.isNumber(0)) ::Php::Value::operator=(input.number(0));
            else if (input.isNull(0))   ::Php::Value::operator=(nullptr);
            else                        ::Php::Value::operator=(input.string(0));
        }
        else
        {
            // turn the variable into an array
            setType(::Php::Type::Array);

            // loop over all the fields and add them one by one
            for (size_t i = 0; i < input.fields(); ++i)
            {
                // assign the value
                if      (input.isNumber(i)) set(i, input.number(i));
                else if (input.isNull(i))   set(i, nullptr);
                else                        set(i, input.string(i));
            }
        }
    }
    
    /**
     *  Destructor
     */
    virtual ~Php() = default;
};

/**
 *  Class to convert a tuple into a json representation
 */
class Json : public JSON::Array
{
public:
    /**
     *  Constructor
     *  @param  input
     *  @return JSON::Object
     */
    Json(const Yothalot::Tuple &input)
    {
        // loop over all the fields and add them one by one
        for (size_t i = 0; i < input.fields(); ++i)
        {
            // determine the type to add
            if      (input.isNumber(i)) append(input.number(i));
            else if (input.isNull(i))   append(nullptr);
            else                        append(input.string(i));
        }
    }
    
    /**
     *  Destructor
     */
    virtual ~Json() = default;
};

/**
 *  End of namespace
 */
}

