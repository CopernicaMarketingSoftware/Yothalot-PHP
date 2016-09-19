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
#include "raceresult.h"
#include "mapreduceresult.h"
#include "error.h"
#include "taskresult.h"
#include <iostream>

/**
 *  Different error types
 */
using MapReduceError    =   Error<MapReduceResult>;
using RaceError         =   Error<RaceResult>;
using TaskError         =   Error<TaskResult>;

/**
 *  Class definition
 */
class Job :
    public Php::Base,
    public Php::Serializable,
    public TupleHelper
{
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
        if (!algo.instanceOf("Yothalot\\MapReduce") && !algo.instanceOf("Yothalot\\Race") && !algo.instanceOf("Yothalot\\Task")) throw Php::Exception("Connection is not an instance of Yothalot\\MapReduce, Yothalot\\Race or Yothalot\\Task.");

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
     *  Set the split-size to be used for input used
     *  in the mapper task.
     *
     *  @param  params  PHP input parameters
     *  @return Same object for chaining, or a nullptr in case of failure
     */
    Php::Value splitsize(Php::Parameters &params)
    {
        // pass this to the implementation
        if (!_impl->splitsize(params[0].numericValue())) return nullptr;

        // allow chaining
        return this;
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
        if (!_impl->maxfinalizers(std::max(0L, params[0].numericValue()))) return nullptr;

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
        // extract the number, old behavior is all the same value
        int64_t mapperfiles = params[0].numericValue();
        int64_t reducerfiles = params.size() >= 2 ? params[1].numericValue() : mapperfiles;
        int64_t finalizerfiles = params.size() >= 3 ? params[2].numericValue() : mapperfiles;

        // pass on to the implementation object
        if (!_impl->maxfiles(mapperfiles, reducerfiles, finalizerfiles)) return nullptr;

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
        // extract the number, old behavior is all the same value
        int64_t mapperbytes = params[0].numericValue();
        int64_t reducerbytes = params.size() >= 2 ? params[1].numericValue() : mapperbytes;
        int64_t finalizerbytes = params.size() >= 3 ? params[2].numericValue() : mapperbytes;

        // pass on to the implementation object
        if (!_impl->maxbytes(mapperbytes, reducerbytes, finalizerbytes)) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Limit the amount of records
     *
     *  @param  params  PHP input parameters
     *  @return Same object for chaining or nullptr on failure
     */
    Php::Value maxrecords(Php::Parameters &params)
    {
        // pass on to implementation object
        if (!_impl->maxrecords(params[0])) return nullptr;

        // success, allow chaining
        return this;
    }

    /**
     *  Add data to this job
     *  @param  params  PHP input parameters
     *  @return         Result is the object for chaining or nullptr on failure
     */
    Php::Value add(Php::Parameters &params)
    {
        if(_impl->isMapReduce())
        {
            if (params.size() >= 2)
            {
                // create the key and the value from the parameters
                auto key = toTuple(params[0]);
                auto value = toTuple(params[1]);
    
                // get the possible server
                const char* server = params.size() >= 3 ? params[2].rawValue() : nullptr;
    
                // pass on to the implementation object
                if (!_impl->add(key, value, server)) return nullptr;
    
                // allow chaining
                return this;
            }
            else
            {
                // i guess we failed now
                return nullptr;
            }
        }
        else
        {
            // it is a race and we only use the first parameter
            // serialize and base64 encode the data to ensure that no null character appear in it
            auto data = Php::call("base64_encode", Php::call("serialize", params[0])).stringValue();
            
            // else it is a race job to which we can add data
            if (!_impl->add(data)) return nullptr;

            // allow chaining
            return this;
        }
    }

    /**
     *  Add data to this job
     *  @param  params  PHP input parameters
     *  @return         Result is the object for chaining or nullptr on failure
     */
    Php::Value map(Php::Parameters &params)
    {
        // create the key and the value from the parameters
        auto key = toTuple(params[0]);
        auto value = toTuple(params[1]);

        // get the possible server
        const char* server = params.size() >= 3 ? params[2].rawValue() : nullptr;

        // immediately redirect
        if (!_impl->map(key, value, server)) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Set the local property
     *  @param  params      PHP input parameters
     *  @return             Result is the object for chaining or nullptr on failure
     */
    Php::Value local(Php::Parameters &params)
    {
        // pass on to the implementation object
        if (!_impl->local(params[0].boolValue())) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Flush the file, causing a new output file to be created.
     *  @return             Result is the object for chaining or nullptr on failure
     */
    Php::Value flush()
    {
        // pass on to the implementation object
        if (!_impl->flush()) return nullptr;

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
        // get the params in c++
        const char *filename = params[0].rawValue();
        int64_t start = params.size() >= 2 ? params[1].numericValue() : 0;
        int64_t size = params.size() >= 3 ? params[2].numericValue() : 0;
        bool remove = params.size() >= 4 ? params[3].boolValue() : false;
        const char *server = params.size() >= 5 ? params[4].rawValue() : nullptr;

        // call the file function in the implementation
        if (!_impl->file(filename, (size_t)start, (size_t)size, remove, server)) return nullptr;

        // allow chaining
        return this;
    }

    /**
     *  Add a directory to this job
     *  @param  params  PHP input parameters
     *  @return         the object for chaining or a nullptr on failure
     */
    Php::Value directory(Php::Parameters &params)
    {
        // if we don't have any parameters we act as a get operation
        if (params.empty()) return _impl->directory() ? _impl->directory() : nullptr;

        // get the params in c++
        auto dirname = params[0].rawValue();
        auto remove = params.size() >= 1 ? params[1].boolValue() : false;
        auto server = params.size() >= 2 ? params[2].rawValue() : "";

        // call the file function in the implementation
        if (!_impl->directory(dirname, remove, server)) return nullptr;

        // allow chaining
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
     *  Execute this job, but do not detach from it (we can still wait for it)
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
        // did the job execute successfully?
        bool success = _impl->wait();

        // what algorithm did we just wait for?
        switch (_impl->algorithm())
        {
            case Algorithm::race:
                if (success)    return Php::Object{ "Yothalot\\RaceResult",         new RaceResult      { _impl->result()   }};
                else            return Php::Object{ "Yothalot\\RaceError",          new RaceError       { _impl->result()  }};
            case Algorithm::mapreduce:
                if (success)    return Php::Object{ "Yothalot\\MapReduceResult",    new MapReduceResult { _impl->result()   }};
                else            return Php::Object{ "Yothalot\\MapReduceError",     new MapReduceError  { _impl->result()  }};
            case Algorithm::job:
                if (success)    return Php::Object{ "Yothalot\\TaskResult",         new TaskResult      { _impl->result()   }};
                else            return Php::Object{ "Yothalot\\TaskError",          new TaskError       { _impl->result()  }};
            default:            return nullptr;
        }
    }
    
    /**
     *  Is the job ready yet?
     *  @return bool
     */
    bool ready() const
    {
        // pass on to the implementation
        return _impl->ready();
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

    /**
     *  The underlying connection
     *  @return Core
     */
    const std::shared_ptr<Core> &core() const
    {
        return _impl->core();
    }
};
