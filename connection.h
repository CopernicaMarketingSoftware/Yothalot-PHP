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
#include "core.h"

/**
 *  Class definition
 */
class Connection :
    public Php::Base {
private:
    /**
     *  Shared pointer to the core
     *  @var std::shared_ptr<Core>
     */
    std::shared_ptr<Core> _core;

public:
    /**
     *  Constructor
     */
    Connection() {}

    /**
     *  Destructor
     */
    virtual ~Connection()  {};

    /**
     *  The php constructor
     *  @param  params
     */
    void __construct(Php::Parameters &params)
    {
        // we need all parameters
        Php::Value param = params.size() == 0 ? Php::Object() : params[0];

        // and extract all the optional parameters
        std::string host = (param.contains("host") ? param["host"] : "localhost");
        std::string user = (param.contains("user") ? param["user"] : "guest");
        std::string password = (param.contains("password") ? param["password"] : "guest");
        std::string vhost = (param.contains("vhost") ? param["vhost"] : "/");
        std::string exchange = (param.contains("exchange") ? param["exchange"] : "");
        std::string mapreduce = (param.contains("mapreduce") ? param["mapreduce"] : "mapreduce");
        std::string races = (param.contains("races") ? param["races"] : "races");
        std::string jobs = (param.contains("jobs") ? param["jobs"] : "jobs");

        // creating a connection could throw
        try
        {
            // create the actual connection
            _core= std::make_shared<Core>(host, user, password, vhost, exchange, mapreduce, races, jobs);
        }
        catch (const std::runtime_error &error)
        {
            // convert C++ exception into a PHP exception
            throw Php::Exception(error.what());
        }
    }

    /**
     *  Flush the connection
     *  This runs the event loop until everything has been sent
     */
    void flush()
    {
        // call the flush method on the core
        _core->flush();
    }

    /**
     *  Retrieve the core
     *  @return std::shared_ptr<Core>
     */
    const std::shared_ptr<Core> &core() const { return _core; }
};
