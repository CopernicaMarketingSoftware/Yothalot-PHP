/**
 *  Writer.h
 *
 *  The writer class.
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
#include "tuple.h"

/**
 *  Class definition
 */
class Writer : public Php::Base
{
private:
    /**
     *  The actual underlying writer by reference
     */
    Yothalot::Writer &_writer;

public:
    /**
     *  Constructor
     */
    Writer(Yothalot::Writer &writer) : _writer(writer) {}

    /**
     *  Destructor
     */
    virtual ~Writer() {};

    /**
     *  Emit a value
     */
    void emit(Php::Parameters &params)
    {
        // retrieve the value
        Php::Value value = params[0];

        // and pass the value as a tuple onto the writer
        _writer.emit(Tuple::Yothalot(value));
    }
};