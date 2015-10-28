/**
 *  Stats.h
 *  Statistics class for mapper, reducer, and finalizer results
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
#include "json/object.h"
#include "datastats.h"

/**
 *  class definition
 */

class Stats : public Php::Base
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
     */
    Stats(const JSON::Object &json) : _json(json) {}

    /**
     *  Virtual destructor
     */
    virtual ~Stats() {}

    /**
     *  get the first time value started
     *  @return Php::Value
     */
    Php::Value first() const
    {
        return _json.decimal("first");
    }

    /**
     *  get the last time of value
     *  @return Php::Value
     */
    Php::Value last() const
    {
        return _json.decimal("last");
    }

    /**
     *  get the last time of value
     *  @return Php::Value
     */
    Php::Value finished() const
    {
        return _json.decimal("finished");
    }

    /**
     *  get the last time of value
     *  @return Php::Value
     */
    Php::Value fastest() const
    {
        return _json.decimal("fastest");
    }

    /**
     *  get the last time of value
     *  @return Php::Value
     */
    Php::Value slowest() const
    {
        return _json.decimal("slowest");
    }

    /**
     *  get the last time of value
     *  @return Php::Value
     */
    Php::Value processes() const
    {
        return _json.integer("processes");
    }

    /**
     *  get the last time of value
     *  @return Php::Value
     */
    Php::Value runtime() const
    {
        return _json.decimal("runtime");
    }

    /**
     *  get the last time of value
     *  @return Php::Value
     */
    Php::Value input() const
    {
        return Php::Object("Yothalot\\DataStats", new DataStats(_json.object("input")));
    }

    /**
     *  get the last time of value
     *  @return Php::Value
     */
    Php::Value output() const
    {
        return Php::Object("Yothalot\\DataStats", new DataStats(_json.object("output")));
    }
};
 
