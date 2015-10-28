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
#include "base.h"

/**
 *  Class definition
 */
class Path : public Php::Base {
private:
    /**
     *  The underlying fullname class
     */
    std::unique_ptr<Yothalot::Fullname> _fullname;

public:
    /**
     *  Constructor
     */
    Path() {}

    /**
     *  Destructor
     */
    virtual ~Path() {};

    /**
     *  Php constructor
     *  @param  params
     */
    void __construct(Php::Parameters &params)
    {
        // construct the fullname class
        _fullname.reset(new Yothalot::Fullname(base(), params[0].stringValue()));

        // check if we're valid, throw otherwise
        if (!_fullname || !(*_fullname)) throw Php::Exception(params[0].stringValue() + " is not on a glusterfs mount");
    }

    /**
     *  Return the absolute path
     *  @return Php::Value
     */
    Php::Value absolute() const
    {
        // return the fullname
        return _fullname->full();
    }

    /**
     *  Return the relative path
     *  @return Php::Value
     */
    Php::Value relative() const
    {
        // return the relative path
        return _fullname->relative();
    }

    /**
     *  Cast to a string
     *  @return const char *
     */
    const char *__toString() const
    {
        return _fullname->absolute();
    }
};

