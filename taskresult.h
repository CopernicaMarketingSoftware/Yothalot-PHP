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
#include "winner.h"

/**
 *  Class definition
 */
class TaskResult : public Php::Base
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
    TaskResult(const JSON::Object &output) : _json(output) {}

    /**
     *  Constructor
     *  @param  output
     */
    TaskResult(const std::shared_ptr<JSON::Object> &output) : _json(*output) {}

    /**
     *  Destructor
     */
    virtual ~TaskResult() {}

    /**
     *  Get the time when the job is started
     *  @return Php::Value
     */
    Php::Value started() const
    {
        return _json.decimal("started");
    }

    /**
     *  Get the time when the job is finished
     *  @return Php::Value
     */
    Php::Value finished() const
    {
        return _json.decimal("finished");
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
     *  Get the result, only used for races and regular jobs
     *  @return Php::Value
     */
    Php::Value result() const
    {
        // unserialize the base64 encoded object from stdout
        return Php::call("unserialize", Php::call("base64_decode", _json.c_str("stdout")));
    }
};

