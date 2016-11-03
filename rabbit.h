/**
 *  Rabbit.h
 *
 *  Wrap a rabbitmq connection
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
#include <amqpcpp.h>
#include <copernica/nosql.h>
#include "json/object.h"
#include "descriptors.h"
#include "loop.h"
#include "tcphandler.h"

/**
 *  Class definition
 */
class Rabbit : private AMQP::TcpHandler, public TcpHandler
{
private:
    /**
     *  The AMQP address
     *  @var AMQP::Address
     */
    AMQP::Address _address;
    
    /**
     *  The name of the exchange to which messages are published
     *  @var std::string
     */
    std::string _exchange;
    
    /**
     *  The name of the queues for mapreduce, regular and race jobs
     *  @var std::string
     */
    std::string _mapreduce;
    std::string _races;
    std::string _jobs;

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

        // create the connection (it will the stored as a member in the onConnected() method)
        auto *connection = new AMQP::TcpConnection(this, _address);

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
     *  @param  address             rabbitmq address
     *  @param  exchange            rabbitmq exchange
     *  @param  mapreduce           queue for mapreduce jobs
     *  @param  races               queue for race jobs
     *  @param  jobs                queue for regular jobs
     *
     *  @throws std::runtime_error
     * 
     *  @todo why std::string?
     */
    Rabbit(std::string address, std::string exchange, std::string mapreduce, std::string races, std::string jobs) : 
        _address(std::move(address)),
        _exchange(std::move(exchange)),
        _mapreduce(std::move(mapreduce)),
        _races(std::move(races)),
        _jobs(std::move(jobs))
    {
        // construct a connection
        auto *connection = new AMQP::TcpConnection(this, _address);
        
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
     *  Destructor
     */
    virtual ~Rabbit()
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
    virtual const Descriptors &descriptors() const override
    {
        // expose member
        return _descriptors;
    }
    
    /**
     *  Method that is called when a filedescriptor becomes active
     *  @param  fd      the filedescriptor that is active
     *  @param  flags   type of activity (readable or writalble)
     */
    virtual void process(int fd, int flags) override
    {
        // pass on to the underlying AMQP connection
        if (_rabbit != nullptr) _rabbit->process(fd, flags);
    }

    /**
     *  Method to publish a JSON encoded message to the mapreduce queue
     *  @param  json        The JSON data to be published
     *  @return bool
     */
    bool mapreduce(const JSON::Object &json)
    {
        // publish to the mapreduce queue
        return publish(_mapreduce, json);
    }

    /**
     *  Method to publish a JSON encoded message to the race queue
     *  @param  json        The JSON data to be published
     *  @return bool
     */
    bool race(const JSON::Object &json)
    {
        // publish to the race queue
        return publish(_races, json);
    }

    /**
     *  Method to publish a JSON encoded message to the jobs queue
     *  @param  json        The JSON data to be published
     *  @return bool
     */
    bool job(const JSON::Object &json)
    {
        // publish to the mapreduce queue
        return publish(_jobs, json);
    }

    /**
     *  Method to publish a JSON encoded message to the queue
     *  @param  queue       The name of the queue to publish to
     *  @param  json        The JSON data to be published
     *  @return bool
     */
    bool publish(const std::string &queue, const JSON::Object &json)
    {
        // create the connection to the RabbitMQ server
        if (!connect()) return false;

        // create temporary channel, so that possible errors do not affect the connection
        AMQP::TcpChannel channel(_rabbit.get());

        // publish the json
        channel.publish(_exchange, queue, json.toString());

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
};
