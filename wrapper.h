/**
 *  Wrapper.h
 *
 *  A wrapper whichs wraps the php object with the mapreduce implementation to the
 *  actual Yothalot::MapReduce class.
 *
 *  @author Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 - 2016 Copernica BV
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
#include "reducer.h"
#include "writer.h"
#include "values.h"
#include "record.h"

/**
 *  Class definition
 */
class Wrapper : public Yothalot::MapReduce
{
private:
    /**
     *  The PHP "Yothalot\MapReduce" object that holds the implementation to all methods
     *  @var Php::Object
     */
    Php::Object _object;

    /**
     *  What sort of implementation do we have?
     *  @var enum
     */
    enum {
        record_reduce,
        map_reduce
    } _type = map_reduce;


    /**
     *  Function to map a record
     *  @param  record
     *  @param  reducer
     */
    virtual void map(const Yothalot::Record &record, Yothalot::Reducer &reducer) override
    {
        // pass to base if there is no custom 
        if (_type != record_reduce) return Yothalot::MapReduce::map(record, reducer);
        
        // prevent PHP exceptions from bubbling up
        try
        {
            // turn the record and the reducer into php objects
            // @todo is it possible to skip turning the record into a shared-ptr?
            Php::Object phprecord("Yothalot\\Record", new Record(std::make_shared<Yothalot::Record>(record)));
            Php::Object phpreducer("Yothalot\\Reducer", new Reducer(reducer));
            
            // forward the map call to php, don't forget to unserialize the data though
            _object.call("map", phprecord, phpreducer);
        }
        catch (const Php::Exception &exception)
        {
            // this is a big problem!
            Php::error << exception.what() << std::flush;
        }
        
    }

    /**
     *  Function to map a key-value to a key/value pair
     *  @param  key
     *  @param  value
     *  @param  reducer
     */
    virtual void map(const Yothalot::Key &key, const Yothalot::Value &value, Yothalot::Reducer &reducer) override
    {
        // prevent PHP exceptions from bubbling up
        try
        {
            // forward the map call to php, don't forget to unserialize the data though
            _object.call("map", Tuple::Php(key), Tuple::Php(value), Php::Object("Yothalot\\Reducer", new Reducer(reducer)));
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
        // prevent PHP exceptions from bubbling up
        try
        {
            // forward the reduce call to php, the tuple will only convert the tuple to a Php::Array
            _object.call("reduce", Tuple::Php(key), Php::Object("Yothalot\\Values", new Values(values)), Php::Object("Yothalot\\Writer", new Writer(writer)));
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
        // prevent PHP exceptions from bubbling up
        try
        {
            // forward the write call to php
            _object.call("write", Tuple::Php(key), Tuple::Php(value));
        }
        catch (const Php::Exception &exception)
        {
            // this is a big problem!
            Php::error << exception.what() << std::flush;
        }
    }

public:
    /**
     *  Constructor
     *  @param  object      The PHP object with the implementation
     */
    Wrapper(Php::Object &&object) : _object(std::move(object))
    {
        // make sure we're the correct type
        if      (_object.instanceOf("Yothalot\\MapReduce"))     _type = map_reduce;
        else if (_object.instanceOf("Yothalot\\RecordReduce"))  _type = record_reduce;
        
        // report an error
        else Php::error << "Failed to unserialize to Yothalot\\MapReduce object" << std::flush;
    }

    /**
     *  Destructor
     */
    virtual ~Wrapper() = default;
};

