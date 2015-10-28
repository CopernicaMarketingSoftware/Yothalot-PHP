/**
 *  Reducer.h
 *
 *  The reducer class.
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

#include "tuplehelper.h"

/**
 *  Class definition
 */
class Reducer :
    public Php::Base,
    public TupleHelper {
private:
    /**
     *  The underlying reducer by reference
     */
    Yothalot::Reducer &_reducer;

public:
    /**
     *  Constructor
     */
    Reducer(Yothalot::Reducer &reducer) : _reducer(reducer) {};

    /**
     *  Destructor
     */
    virtual ~Reducer() {};

    /**
     *  Get the size of the log file in bytes
     */
    void emit(Php::Parameters &params)
    {
        // retrieve the key and value
        Php::Value key = params[0];
        Php::Value value = params[1];

        // pass the key and value to the actual reducer
        _reducer.emit(toTuple(key), toTuple(value));
    }
};