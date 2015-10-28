/**
 *  IOStats.h
 *
 *  Statistics class for mapper, reducer, and finalizer results
 *
 *  @author Aljar Meesters <aljar.meesters@copernica.com>
 *  @copyright Copernica BV 2015
 */
 
/**
 *  include guard
 */
#pragma once

/**
 *  dependencies
 */
#include <phpcpp.h>
#include "json/object.h"

/**
 *  class definition
 */

class DataStats : public Php::Base
{
private:
    /**
     *  json that holds the information
     *  @var JSON::Object
     */
    JSON::Object _json;

public:
    /**
     *  Constructor
     *  @param output
     *  @param type
     */
    DataStats(const JSON::Object &json) :
        _json(json) {}

    /**
     *  Virtual destructor
     */
    virtual ~DataStats() {}

    /**
     *  get the number of files
     *  @return Php::Value
     */
    Php::Value files() const
    {
        return _json.integer("files");
    }

    /**
     *  get the number of bytes
     *  @return Php::Value
     */
    Php::Value bytes() const
    {
        return _json.integer("bytes");
    }
};
 
