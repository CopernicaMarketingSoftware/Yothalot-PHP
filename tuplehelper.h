/**
 *  TupleHelper.h
 *
 *  A simple tuple helper, inherit from this class to get the toTuple and fromTuple method.
 *
 *  @author    Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 Copernica BV
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

/**
 *  Class definition
 */
class TupleHelper {
protected:
    /**
     *  Inherit me please..
     */
    TupleHelper() {}

    /**
     *  Turn a php value into a log tuple
     *  @param  value
     *  @return Yothalot::Tuple
     */
    Yothalot::Tuple toTuple(const Php::Value &value) const
    {
        // if we're numeric or a string we just return a tuple with a single value
        if (value.isNumeric())     return { value.numericValue() };
        else if (value.isString()) return { value.stringValue() };
        else if (value.isArray() || value.isObject())
        {
            // if we're an array or object we actually create a tuple
            Yothalot::Tuple output;

            // we start looping over it
            for (auto iter : value)
            {
                // we're going to add numeric values as numbers, everything else as strings
                if (iter.second.isNumeric())   output.add(iter.second.numericValue());
                else if (iter.second.isNull()) output.add(nullptr);
                else                           output.add(iter.second.stringValue());
            }

            // and we return this tuple of course
            return output;
        }

        // if you're anything else we return an empty tuple
        return {};
    }

    /**
     *  Turn a log tuple into a Php::Value
     *  @param  tuple
     *  @return Php::Value
     */
    Php::Value fromTuple(const Yothalot::Tuple &input) const
    {
        // if we have a single field we're not building an array
        if (input.fields() == 1)
        {
            if (input.isNumber(0))       return input.number(0);
            else if (input.isNull(0))    return nullptr;
            else                         return input.string(0);
        }

        // otherwise we're building an array
        Php::Value output(Php::Type::Array);

        // loop over all the fields and add them one by one
        for (size_t i = 0; i < input.fields(); ++i)
        {
            if (input.isNumber(i))    output.set(i, input.number(i));
            else if (input.isNull(i)) output.set(i, nullptr);
            else                      output.set(i, input.string(i));
        }

        // return the output
        return output;
    }
};