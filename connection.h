/**
 *  Connection.h
 *
 *  PHP class that contains all data about the rabbitmq and nosql settings
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

#include "tuplehelper.h"
#include "rabbit.h"
#include "cache.h"

/**
 *  Class definition
 */
class Connection :
    public Php::Base,
    public Php::Serializable {
private:
    /**
     *  Shared pointer to the rabbit
     *  @var std::shared_ptr<Rabbit>
     */
    std::shared_ptr<Rabbit> _rabbit;
    
    /**
     *  Shared pointer to the cache settings
     *  @var std::shared_ptr<Cache>
     */
    std::shared_ptr<Cache> _cache;

    /**
     *  JSON object with all the nosql and rabbitmq settings
     */
    JSON::Object _json;


public:
    /**
     *  Constructor
     */
    Connection() {}

    /**
     *  Destructor
     */
    virtual ~Connection() = default;

    /**
     *  The php constructor
     *  @param  params
     */
    void __construct(Php::Parameters &params)
    {
        // we need all parameters
        Php::Value param = params.size() == 0 ? Php::Object() : params[0];

        // and extract all the optional parameters for rabbitmq
        std::string address     = (param.contains("address")    ? param["address"]      : Php::ini_get("yothalot.address")     .stringValue());
        std::string exchange    = (param.contains("exchange")   ? param["exchange"]     : Php::ini_get("yothalot.exchange")    .stringValue());
        std::string mapreduce   = (param.contains("mapreduce")  ? param["mapreduce"]    : Php::ini_get("yothalot.mapreduce")   .stringValue());
        std::string races       = (param.contains("races")      ? param["races"]        : Php::ini_get("yothalot.races")       .stringValue());
        std::string jobs        = (param.contains("jobs")       ? param["jobs"]         : Php::ini_get("yothalot.jobs")        .stringValue());

        // extract the optional parameters for nosql
        std::string cache       = (param.contains("cache")      ? param["cache"]        : Php::ini_get("yothalot.cache")       .stringValue());
        size_t      maxcache    = (param.contains("maxcache")   ? param["maxcache"]     : Php::ini_get("yothalot.maxcache")    .numericValue());
        time_t      ttl         = (param.contains("ttl")        ? param["ttl"]          : Php::ini_get("yothalot.ttl")         .numericValue());

        // store all properties in the JSON
        _json.set("address", address);
        _json.set("exchange", exchange);
        _json.set("mapreduce", mapreduce);
        _json.set("races", races);
        _json.set("jobs", jobs);
        _json.set("cache", cache);
        _json.set("maxcache", (int64_t)maxcache);
        _json.set("ttl", (int64_t)ttl);

        // creating a connection could throw
        try
        {
            // create the actual rabbitmq and nosql connections
            _rabbit = std::make_shared<Rabbit>(std::move(address), std::move(exchange), std::move(mapreduce), std::move(races), std::move(jobs));
        }
        catch (const std::runtime_error &error)
        {
            // convert C++ exception into a PHP exception
            throw Php::Exception(std::string("rabbitmq error: ") + error.what());
        }
        
        // prevent exceptions for nosql errors
        try
        {
            // create the nosql error
            _cache = std::make_shared<Cache>(std::move(cache), maxcache, ttl);
        }
        catch (const std::runtime_error &error)
        {
            // convert C++ exception into a PHP exception
            throw Php::Exception(std::string("cache error: ") + error.what());
        }
    }

    /**
     *  Flush the connection
     *  This runs the event loop until everything has been sent
     */
    void flush()
    {
        // call the flush method on the rabbitmq connection
        _rabbit->flush();
    }

    /**
     *  Retrieve the rabbit object
     *  @return std::shared_ptr<Rabbit>
     */
    const std::shared_ptr<Rabbit> &rabbit() const { return _rabbit; }

    /**
     *  Retrieve the cache object
     *  @return std::shared_ptr<Cache>
     */
    const std::shared_ptr<Cache> &cache() const { return _cache; }

    /**
     *  Method to serialize the object
     *
     *  This method should return a string representation of the object that
     *  can be passed to the unserialize() method and that will revive the object
     *
     *  @return std::string
     */
    virtual std::string serialize() override
    {
        return _json;
    }

    /**
     *  Unserialize the object
     *
     *  This method is called as an alternative __construct() method to initialize
     *  the object. The passed in string parameter in in the format earlier returned
     *  by a call to serialize()
     *
     *  @param  input           String to parse
     *  @param  size            Size of the string
     */
    virtual void unserialize(const char *input, size_t size) override
    {
        // parse the json object
        JSON::Object json(input, size);

        // and extract all the optional parameters
        std::string address = (json.contains("address") ? json.c_str("address") : Php::ini_get("yothalot.address"));
        std::string exchange = (json.contains("exchange") ? json.c_str("exchange") : Php::ini_get("yothalot.exchange"));
        std::string mapreduce = (json.contains("mapreduce") ? json.c_str("mapreduce") : Php::ini_get("yothalot.mapreduce"));
        std::string races = (json.contains("races") ? json.c_str("races") : Php::ini_get("yothalot.jobs"));
        std::string jobs = (json.contains("jobs") ? json.c_str("jobs") : Php::ini_get("yothalot.jobs"));
        std::string cache = (json.contains("cache") ? json.c_str("cache") : Php::ini_get("yothalot.cache"));
        size_t maxcache = (json.contains("maxcache") ? json.integer("maxcache") : Php::ini_get("yothalot.maxcache"));
        time_t ttl = (json.contains("ttl") ? json.integer("ttl") : Php::ini_get("yothalot.ttl"));

        // creating a connection could throw
        try
        {
            // create the actual connections
            _rabbit = std::make_shared<Rabbit>(std::move(address), std::move(exchange), std::move(mapreduce), std::move(races), std::move(jobs));
            _cache = std::make_shared<Cache>(std::move(cache), maxcache, ttl);
        }
        catch (const std::runtime_error &error)
        {
            // convert C++ exception into a PHP exception
            throw Php::Exception(error.what());
        }
    }
};
