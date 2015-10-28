/**
 *  ValuesIterator.h
 *
 *  Class to iterate over values
 *
 *  @author Toon Schoenmakers <toon.schoenmakers@copernica.com>
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

#include "tuplehelper.h"

/**
 *  Forward declaration
 */
class Values;

/**
 *  Class definition
 */
class ValuesIterator :
    public Php::Iterator,
    public TupleHelper
{
private:
    /**
     *  The values belonging to this iterator.
     *  @var Yothalot::Values
     */
    Yothalot::Values &_values;

    /**
     *  Amount of times the next method has been called
     *  @var int64_t
     */
    int64_t _counter = 0;

public:
    /**
     *  Constructor
     *  @param  values      The values to iterate over.
     */
    ValuesIterator(Values *values);

    /**
     *  Destructor
     */
    virtual ~ValuesIterator() {}

    /**
     *  Is the iterator on a valid position
     *  @return bool
     */
    virtual bool valid() override
    {
        // cast whether or not we still have a tuple
        return (bool) _values;
    }

    /**
     *  The value at the current position
     *  @return Php::Value
     */
    virtual Php::Value current() override
    {
        // must be set
        if (!_values) return nullptr;

        // construct the tuple
        return fromTuple(*_values);
    }

    /**
     *  The key at the current position
     *  @return Value
     */
    virtual Php::Value key() override
    {
        // return counter
        return _counter;
    }

    /**
     *  Move to the next position
     */
    virtual void next() override
    {
        // increment the values
        ++_values;

        // increment the counter as well
        ++_counter;
    }

    /**
     *  Rewind the iterator to the front position
     */
    virtual void rewind() override
    {
        // empty, because the nature of the values object is only forward iterable,
        // since the input may be _very_ large.
    }
};

