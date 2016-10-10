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
#include <amqpcpp.h>

#include "json/object.h"
#include "tuplehelper.h"
#include "descriptors.h"
#include "loop.h"

/**
 *  Class definition
 */
class Core : private AMQP::TcpHandler
{
private:
    /**
     *  JSON object holding all properties
     *  @var JSON::Object
     */
    JSON::Object _json;

    /**
     *  The file descriptors used by the connection
     *  @var Descriptors
     */
    Descriptors _descriptors;

    /**
     *  The underlying TCP connection
     *  @var AMQP::TcpConnection
     */
    std::unique_ptr<AMQP::TcpConnection> _rabbit;

    /**
     *  The error that was discovered
     *  @var std::string
     */
    std::string _error;
    
    /**
     *  The nosql connection
     *  @var NoSql::Connection
     */
    std::unique_ptr<Copernica::NoSql::Connection> _nosql;

    /**
     *  Called when connection is in error state
     *  @param  connection      The connection that entered the error state
     *  @param  message         Error message
     */
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        // store the error
        _error.assign(message);

        // reset the connection
        _rabbit = nullptr;
    }

    /**
     *  Called when connection is established
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(AMQP::TcpConnection *connection) override
    {
        // store the connection
        _rabbit.reset(connection);
    }

    /**
     *  Callback called when connection is closed
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(AMQP::TcpConnection *connection) override
    {
        // reset connection
        _rabbit = nullptr;
    }

    /**
     *  Monitor a filedescriptor for readability or writability
     *  @param  connection  The TCP connection object that is reporting
     *  @param  fd          The filedescriptor to be monitored
     *  @param  flags       Should the object be monitored for readability or writability?
     */
    virtual void monitor(AMQP::TcpConnection *connection, int fd, int flags) override
    {
        // add the filedescriptor to the loop
        _descriptors.add(fd, flags);
    }

    /**
     *  Create the AMQP connection
     *  @return bool
     */
    bool connect()
    {
        // not necessary if already connected
        if (_rabbit) return true;

        // the connection address
        AMQP::Address address(_json.c_str("address"));

        // create the connection (it will the stored as a member in the onConnected() method)
        auto *connection = new AMQP::TcpConnection(this, address);

        // keep running the event loop, until the connection is valid
        while (!_rabbit && _error.empty())
        {
            // construct the event loop
            Loop loop(_descriptors);
            
            // run it
            loop.step(connection);
        }
        
        // was the connection stored as member?
        if (_rabbit) return true;

        // report error to PHP space
        if (!_error.empty()) Php::warning << _error << std::endl;

        // done
        return false;
    }

public:
    /**
     *  Constructor
     *
     *  Watch out: throws an exception when connection could not be established
     *
     *  @param  address
     *  @param  exchange
     *  @param  mapreduce
     *  @param  races
     *  @param  jobs
     *
     *  @throws std::runtime_error
     * 
     *  @todo why std::string?
     */
    Core(const std::string &address, const std::string &exchange, const std::string &mapreduce, const std::string &races, const std::string &jobs)
    {
        // store all properties in the JSON
        _json.set("address", address);
        _json.set("exchange", exchange);
        _json.set("mapreduce", mapreduce);
        _json.set("races", races);
        _json.set("jobs", jobs);
        
        // construct a connection
        auto *connection = new AMQP::TcpConnection(this, AMQP::Address(address));
        
        // keep running the event loop, until the connection is valid
        while (!_rabbit && _error.empty())
        {
            // construct the event loop
            Loop loop(_descriptors);
            
            // run it
            loop.step(connection);
        }
        
        // was the connection stored as member?
        if (_rabbit) return;
        
        // we apparently have a failure
        delete connection;
        
        // connection was reset, this means that the onError() method was called
        throw std::runtime_error(_error);
    }

    /**
     *  Constructor based on serialized JSON data
     *  @param  data
     */
    Core(const JSON::Object &object) :
        _json(object), _rabbit(nullptr) {}

    /**
     *  Destructor
     */
    virtual ~Core()
    {
        // nothing to do for us when we're not connected
        if (!_rabbit) return;

        // close the connection
        _rabbit->close();

        // create the event loop
        Loop loop(_descriptors);

        // run the loop (it only contains the connection, so it runs until the connection is closed)
        loop.run(_rabbit.get());
    };

    /**
     *  Expose the filedescriptors
     *  @return Descriptors
     */
    const Descriptors &descriptors() const
    {
        // expose member
        return _descriptors;
    }

    /**
     *  Method to publish a JSON encoded message to the mapreduce queue
     *  @param  json        The JSON data to be published
     *  @return bool
     */
    bool mapreduce(const JSON::Object &json)
    {
        // publish to the mapreduce queue
        return publish(_json.c_str("mapreduce"), json);
    }

    /**
     *  Method to publish a JSON encoded message to the race queue
     *  @param  json        The JSON data to be published
     *  @return bool
     */
    bool race(const JSON::Object &json)
    {
        // publish to the race queue
        return publish(_json.c_str("races"), json);
    }

    /**
     *  Method to publish a JSON encoded message to the jobs queue
     *  @param  json        The JSON data to be published
     *  @return bool
     */
    bool job(const JSON::Object &json)
    {
        // publish to the mapreduce queue
        return publish(_json.c_str("jobs"), json);
    }

    /**
     *  Method to publish a JSON encoded message to the queue
     *  @param  queue       The name of the queue to publish to
     *  @param  json        The JSON data to be published
     *  @return bool
     */
    bool publish(const char *queue, const JSON::Object &json)
    {
        // create the connection to the RabbitMQ server
        if (!connect()) return false;

        // create temporary channel, so that possible errors do not affect the connection
        AMQP::TcpChannel channel(_rabbit.get());

        // publish the json
        channel.publish(_json.c_str("exchange"), queue, json.toString());

        // done
        return true;
    }

    /**
     *  Flush the connection
     *  This runs the event loop until everything has been sent
     */
    void flush()
    {
        // flush is pointless without a connection
        if (!_rabbit) return;

        // create an event loop with just these file descriptors
        Loop loop(_descriptors);

        // step through the loop as long as we have active channels
        // we're working in such a way that all the channels drop away the second
        // they're no longer needed, meaning that we've pushed/retrieved everything
        // from rabbitmq the second we have no channels left
        while (_rabbit->channels() > 0) loop.step(_rabbit.get());
    }

    /**
     *  Expose the connection 
     *  Returns nullptr on error
     *  @return AMQP::TcpConnection
     */
    AMQP::TcpConnection *connection()
    {
        // connect object
        if (!connect()) return nullptr;

        // retrieve the connection
        return _rabbit.get();
    }
    
    /**
     *  Expose the nosql cache
     *  @return Copernica::NoSql::Connection
     */
    Copernica::NoSql::Connection *nosql()
    {
        // do we already have a nosql connection?
        if (_nosql != nullptr) return _nosql.get();
        
        // prevent exceptions
        try
        {
            // create the connection
            auto *connection = new Copernica::NoSql::Connection(Php::ini_get("yothalot.cache"));
            
            // store in the smart pointer
            _nosql.reset(connection);
            
            // done
            return connection;
        }
        catch (const std::runtime_error &error)
        {
            // report error
            Php::warning << error.what() << std::flush;
            
            // no connection available
            return nullptr;
        }
    }

    /**
     *  Retrieve the JSON representation of this object
     *  @return JSON::Object
     */
    const JSON::Object &json() const
    {
        // return the json
        return _json;
    }
};
