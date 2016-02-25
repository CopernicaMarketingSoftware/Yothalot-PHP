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
class ErrorResult : public Php::Base
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
    ErrorResult(const JSON::Object &output) : _json(output) {}

    /**
     *  Constructor
     *  @param  output
     */
    ErrorResult(const std::shared_ptr<JSON::Object> &output) : _json(*output) {}

    /**
     *  Destructor
     */
    virtual ~ErrorResult() {}

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
     *  Get the exectuable that was used for this job
     *  @return Php::Value
     */
    Php::Value executable() const
    {
        return _json.c_str("executable");
    }

    /**
     *  Get the arguments that was used for this job
     *  @return Php::Value
     */
    Php::Value arguments() const
    {
        // get the arguments
        auto args = _json.array("arguments");

        // return as a php value
        return args.phpValue();
    }

    /**
     *  Return the stdin that was passed to this job
     *  @return Php::Value
     */
    Php::Value stdin() const
    {
        return _json.c_str("stdin");
    }

    /**
     *  Return the stdout that was passed to this job
     *  @return Php::Value
     */
    Php::Value stdout() const
    {
        return _json.c_str("stdout");
    }

    /**
     *  Return the stderr that was passed to this job
     *  @return Php::Value
     */
    Php::Value stderr() const
    {
        return _json.c_str("stderr");
    }
};

