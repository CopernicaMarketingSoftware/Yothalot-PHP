/**
 *  Results.h
 * 
 *  PHP results class to retrieve the results from a job
 *
 *  @author Aljar Meesters <aljar.meesters@copernica.com>
 *  @copyright 2015 Copernica BV
 */


/**
 *  include guard
 */
#pragma once


/**
 *  Dependencies
 */
#include "stats.h"

/**
 *  Class definition
 */
class Result : public Php::Base
{
private:
    /**
     *  JSON object holding all properties
     *  @var JSON::Object
     */
    JSON::Object _json;

public:
    /**
     *  Constructor
     *  @param  output
     */
    Result(const JSON::Object &output) : _json(output) {}

    /**
     *  Constructor
     *  @param  output
     */
    Result(const std::shared_ptr<JSON::Object> &output) : _json(*output) {}

    /**
     *  Destructor
     */
    virtual ~Result() {}

    /**
     *  Get the time when the job is started
     *  @return Php::Value
     */
    Php::Value started() const
    {
        return _json.decimal("started");
    }

    /**
     *  Get the total runtime
     *  @return Php::Value
     */
    Php::Value runtime() const
    {
        return _json.decimal("runtime");
    }

    /**
     *  Get the stats of mapper
     *  @return Php::Value
     */
    Php::Value mappers() const
    {
        // return nullptr in case we don't have a mappers json object
        if (!_json.isObject("mappers")) return nullptr;

        // construct and return a Yothalot\Stats object
        return Php::Object("Yothalot\\Stats", new Stats(_json.object("mappers")));
    }

    /**
     *  Get the stats of the reducer
     *  @return Php::Value
     */
    Php::Value reducers() const
    {
        // return nullptr in case we don't have a reducers json object
        if (!_json.isObject("reducers")) return nullptr;

        // construct and return a Yothalot\Stats object
        return Php::Object("Yothalot\\Stats", new Stats(_json.object("reducers")));
    }

    /**
     *  Get the stats of the finalizer
     *  @return Php::Value
     */
    Php::Value finalizers() const
    {
        // return nullptr in case we don't have a finalizers json object
        if (!_json.isObject("finalizers")) return nullptr;

        // construct and return a Yothalot\Stats object
        return Php::Object("Yothalot\\Stats", new Stats(_json.object("finalizers")));
    }

    /**
     *  Get the result, only used for racers atm
     *  @return Php::Value
     */
    Php::Value result() const
    {
        // construct the output value, which is an array
        Php::Value output(Php::Type::Array);

        // turn our json object into a php value and return it
        return _json.object("result").phpValue();
    }
};

