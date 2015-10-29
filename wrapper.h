/**
 *  Wrapper.h
 *
 *  A wrapper whichs wraps the php object with the mapreduce implementation to the
 *  actual Yothalot::MapReduce class.
 *
 *  @author Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <yothalot.h>
#include <phpcpp.h>

#include "tuplehelper.h"
#include "reducer.h"
#include "writer.h"
#include "values.h"

/**
 *  Class definition
 */
class Wrapper :
    public Yothalot::MapReduce,
    public Yothalot::Racer,
    public TupleHelper
{
private:
    /**
     *  The actual C++ task object
     *  @var std::unique_ptr<Yothalot::SubTask>
     */
    std::unique_ptr<Yothalot::SubTask> _task;

    /**
     *  The PHP "Yothalot\MapReduce" or "Yothalot\Racer" object that holds the implementation to all methods
     *  @var Php::Object
     */
    Php::Object _object;

    /**
     *  Function to map a log-record to a key/value pair
     *  @param  record      The record to map
     *  @param  reducer     The result object to which key/value pairs can be mapped
     */
    virtual void map(const char *data, size_t size, Yothalot::Reducer &reducer) override
    {
        // make sure we're not a Racer
        if (!_object.instanceOf("Yothalot\\MapReduce")) Php::error << "MapReduce method called on an object not of type Yothalot\\MapReduce" << std::flush;

        // prevent PHP exceptions from bubbling up
        try
        {
            // forward the map call to php, don't forget to unserialize the data though
            _object.call("map", Php::call("unserialize", Php::call("base64_decode", Php::Value(data, size))), Php::Object("Yothalot\\Reducer", new Reducer(reducer)));
        }
        catch (const Php::Exception &exception)
        {
            // this is a big problem!
            Php::error << exception.what() << std::flush;
        }
    }

    /**
     *  Function to reduce a key that comes with a number of values
     *  @param  key         The key that should be reduced
     *  @param  values      Iteratable object with values that come with this key
     *  @param  writer      The result object to which more key/value pairs can be mapped
     */
    virtual void reduce(const Yothalot::Key &key, const Yothalot::Values &values, Yothalot::Writer &writer) override
    {
        // make sure we're not a Racer
        if (!_object.instanceOf("Yothalot\\MapReduce")) Php::error << "MapReduce method called on an object not of type Yothalot\\MapReduce" << std::flush;

        // prevent PHP exceptions from bubbling up
        try
        {
            // forward the reduce call to php, the tuple will only convert the tuple to a Php::Array
            _object.call("reduce", fromTuple(key), Php::Object("Yothalot\\Values", new Values(values)), Php::Object("Yothalot\\Writer", new Writer(writer)));
        }
        catch (const Php::Exception &exception)
        {
            // this is a big problem!
            Php::error << exception.what() << std::flush;
        }
    }

    /**
     *  Function to write the final result
     *  @param  key         The key for which a single value was found
     *  @param  value       The found value
     */
    virtual void write(const Yothalot::Key &key, const Yothalot::Value &value) override
    {
        // make sure we're not a Racer
        if (!_object.instanceOf("Yothalot\\MapReduce")) Php::error << "MapReduce method called on an object not of type Yothalot\\MapReduce" << std::flush;

        // prevent PHP exceptions from bubbling up
        try
        {
            // forward the write call to php
            _object.call("write", fromTuple(key), fromTuple(value));
        }
        catch (const Php::Exception &exception)
        {
            // this is a big problem!
            Php::error << exception.what() << std::flush;
        }
    }

    /**
     *  Function to map a log-record to a key/value pair
     *  @param  value       The value to map
     *  @param  size        Size of the value
     *  @return std::unique_ptr<Result>    In case you don't have a result please return nullptr
     */
    virtual std::unique_ptr<Yothalot::Racer::Result> process(const char *data, size_t size) override
    {
        // make sure we're not a MapReduce object
        if (!_object.instanceOf("Yothalot\\Racer")) Php::error << "Racer method called on an object not of type Yothalot\\Racer" << std::flush;

        // prevent PHP exceptions from bubbling up
        try
        {
            // forward the map call to php, don't forget to unserialize the data though
            auto output = _object.call("process", Php::call("unserialize", Php::call("base64_decode", Php::Value(data, size))));

            // we only work when we get either an array or an object as output
            if (!output.isArray() && !output.isObject()) return nullptr;

            // create our output
            std::unique_ptr<Yothalot::Racer::Result> result(new Yothalot::Racer::Result());

            // loop over all the elements
            for (auto iter : output) result->put(iter.first.stringValue(), toTuple(iter.second));

            // return the result
            return std::move(result);
        }
        catch (const Php::Exception &exception)
        {
            // this is a big problem!
            Php::error << exception.what() << std::flush;
        }

        // unreachable..
        return nullptr;
    }

public:
    /**
     *  Constructor
     *  @param  object      The PHP object with the implementation
     */
    Wrapper(Php::Object &&object) : _object(std::move(object))
    {
        // make sure we're the correct type
        if (!_object.instanceOf("Yothalot\\MapReduce") && !_object.instanceOf("Yothalot\\Racer")) Php::error << "Failed to unserialize to Yothalot\\MapReduce or Yothalot\\Racer object" << std::flush;
    }

    /**
     *  Destructor
     */
    virtual ~Wrapper() {}
};

