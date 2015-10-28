/**
 *  Job.h
 *
 *  The job class.
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

#include "connection.h"
#include "tuplehelper.h"
#include "reducer.h"
#include "values.h"
#include "writer.h"
#include "directory.h"
#include "jobimpl.h"
#include "serialized.h"
#include "result.h"
#include <iostream>

/**
 *  Class definition
 */
class Job :
    public Php::Base,
    public Php::Serializable,
    public TupleHelper {
private:
    /**
     *  The actual implementation of the job
     * 
     *  This member is either constructed by the __construct() or by the
     *  unserialize() method
     * 
     *  @var JobImpl
     */
    std::unique_ptr<JobImpl> _impl;

public:
    /**
     *  Constructor that creates an initial empty object
     *  The PHP __construct() method will be called right after this constructor)
     */
    Job() {}

    /**
     *  Destructor
     */
    virtual ~Job() {}

    /**
     *  Job PHP constructor
     *  Requires two PHP parameters: a Yothalot\\Connection object, and a
     *  Yothalot\\MapReduce object
     *  @param  params
     */
    void __construct(Php::Parameters &params)
    {
        // check number of parameters
        if (params.size() < 2) throw Php::Exception("Yothalot\\Job constructor requires two parameters");

        // extract the connection and map reduce params
        Php::Value connection = params[0];
        Php::Value algo = params[1];

        // check type of parameters
        if (!connection.instanceOf("Yothalot\\Connection")) throw Php::Exception("Connection is not an instance of Yothalot\\Connection");
        if (!algo.instanceOf("Yothalot\\MapReduce") && !algo.instanceOf("Yothalot\\Racer")) throw Php::Exception("Connection is not an instance of Yothalot\\MapReduce");

        // prevent that exceptions bubble up
        try
        {
            // retrieve the underlying C++ Connection object
            auto *con = (Connection*) connection.implementation();

            // construct the implementation
            _impl.reset(new JobImpl(con->core(), algo));
        }
        catch (const std::runtime_error &error)
        {
            // convert to a PHP exception
            throw Php::Exception(error.what());
        }
    }

    /**
     *  Set the maximum amount of concurrent running processes
     *  @param  params  PHP input parameters
     *  @return         Same object for chaining, or null on error
     */
    Php::Value maxprocesses(Php::Parameters &params)
    {
        // pass this to the implementation
        if (!_impl->maxprocesses(std::max(1L, params[0].numericValue()))) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Set the maximum amount of concurrent running mappers
     *  @param  params  PHP input parameters
     *  @return         Same object for chaining or nullptr on error
     */
    Php::Value maxmappers(Php::Parameters &params)
    {
        // pass this to the implementations
        if (!_impl->maxmappers(std::max(1L, params[0].numericValue()))) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Set the maximum amount of concurrent running reducer
     *  @param  params  PHP input parameters
     *  @return         Same object for chaining or nullptr on error
     */
    Php::Value maxreducers(Php::Parameters &params)
    {
         // pass this to the implementations
        if (!_impl->maxreducers(std::max(1L, params[0].numericValue()))) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Set the maximum amount of concurrent running finalizers
     *  @param  params  PHP input parameters
     *  @return         Same object for chaining or nullptr on error
     */
    Php::Value maxfinalizers(Php::Parameters &params)
    {
        // pass this to the implementation
        if(!_impl->maxfinalizers(std::max(1L, params[0].numericValue()))) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Set the modulo
     *  @param  params  PHP input parameters
     *  @return         Same object for chaining or nullptr on error
     */
    Php::Value modulo(Php::Parameters &params)
    {
        // pass this to the implementation
        if (!_impl->modulo(std::max(1L, params[0].numericValue()))) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Set the maximum amount of files
     *  @param  params  PHP input parameters
     *  @return         NULL on failure, or the same object on success for chaining
     */
    Php::Value maxfiles(Php::Parameters &params)
    {
        // extract the number
        int64_t maxfiles = params[0].numericValue();

        // pass on to the implementation object
        if (!_impl->maxfiles(maxfiles)) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Limit the amount of bytes
     *  @param  params  PHP input parameters
     *  @return         the same object for chaining or nullptr on failure
     */
    Php::Value maxbytes(Php::Parameters &params)
    {
        // extract the number
        int64_t maxbytes = params[0].numericValue();

        // pass on to the implementation
        if (!_impl->maxbytes(maxbytes)) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Add data to this job
     *  @param  params  PHP input parameters
     *  @return         Result is the object for chaining or nullptr on failure
     */
    Php::Value add(Php::Parameters &params)
    {
        // serialize and base64 encode the data to ensure that no null character appear in it
        auto data = Php::call("base64_encode", Php::call("serialize", params[0].stringValue())).stringValue();

        // pass on to the implementation object
        if (!_impl->add(data)) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Add a file to this job
     *  @param  params  PHP input parameters
     *  @return         the object for chaining or a nullptr on failure
     */
    Php::Value file(Php::Parameters &params)
    {
       // serialize and base64 encode the data to ensure that no null character appear in it
        auto data = Php::call("base64_encode", Php::call("serialize", params[0].stringValue())).stringValue();

        // get the filename
        auto filename = params[(params.size() >= 2) ? 1 : 0].stringValue();

        // pass on to the implementation object
        if (!_impl->file(data, filename)) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Add a server to this job
     *  @param  params  PHP input parameters
     *  @return         the object so chaining is possible and nullptr on failure
     */
    Php::Value server(Php::Parameters &params)
    {
        // serialize and base64 encode the data to ensure tha no null character appear in it
        auto data = Php::call("base64_encode", Php::call("serialize", params[0].stringValue())).stringValue();

        // get the servername
        auto servername = params[(params.size() >= 2) ? 1 : 0].stringValue();

        // pass on to the implementation object
        if (!_impl->server(data, servername)) return nullptr;

        // allow chaning
        return this;
    }


    /**
     *  Detach the job -- run it remotely but do not wait for the answer
     *  @return         Result value (true or false)
     */
    Php::Value detach()
    {
        // pass it on to the implementation object
        return _impl->detach();
    }

    /**
     *  Execute this job and wait for it
     *  @return         Result value (true or false)
     */
    Php::Value start()
    {
        // pass it on to the implementation object
        return _impl->start();
    }

    /**
     *  Wait for the job to be ready
     *  @return         Result value (true or false)
     */
    Php::Value wait()
    {
        // pass it on to the implementation object
        if (!_impl->wait()) return nullptr;

        // construct a result object
        return Php::Object("Yothalot\\Result", new Result(_impl->result()));
    }

    /**
     *  Serialize the object to a string
     *  @return std::string
     */
    virtual std::string serialize() override
    {
        // exceptions are thrown when the job is not at all serializable
        // this happens when you serialize an object that does not have
        // a directory on glusterfs, so that the whole reason why one
        // would serialize (to move the job to a different server) is
        // pointless in the first place
        try
        {
            // tell the implementation object to serialize
            Serialized result(_impl.get());

            // done
            return result;
        }
        catch (const std::runtime_error &error)
        {
            // turn the C++ exception into a PHP exception
            throw Php::Exception(error.what());
        }
    }

    /**
     *  Unserialize the object from a string
     *  @param  buffer
     *  @param  size
     */
    virtual void unserialize(const char *buffer, size_t size) override
    {
        // exceptions might be thrown when unserializing, for example
        // when serialized data holds no (valid) directory
        try
        {
            // the serialized object also works the other way round
            Serialized object(buffer, size);

            // reset the _impl unique_ptr to a freshly created JobImpl
            _impl.reset(new JobImpl(object));
        }
        catch (const std::runtime_error &error)
        {
            // turn the C++ error into a PHP error
            throw Php::Exception(error.what());
        }
    }
};
