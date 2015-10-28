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
#include <reactcpp.h>
#include <reactcpp/amqp.h>

#include "json/object.h"
#include "tuplehelper.h"

/**
 *  Class definition
 */
class Core : private React::AMQP::ConnectionHandler
{
private:
    /**
     *  JSON object holding all properties
     *  @var JSON::Object
     */
    JSON::Object _json;

    /**
     *  A thread loop that the actual connection will run on
     *  @var  React::Loop
     */
    React::Loop _loop;

    /**
     *  The underlying AMQP connection
     *  @var  React::AMQP::Connection
     */
    std::unique_ptr<React::AMQP::Connection> _connection;

    /**
     *  The error that was discovered
     *  @var std::string
     */
    std::string _error;

    /**
     *  Called when connection is in error state
     *  @param  connection      The connection that entered the error state
     *  @param  message         Error message
     */
    virtual void onError(React::AMQP::Connection *connection, const char *message) override
    {
        // store the error
        _error.assign(message);

        // reset the connection
        _connection = nullptr;
    }

    /**
     *  Called when connection is established
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(React::AMQP::Connection *connection) override
    {
        // we can stop the event loop for the time being
        _loop.stop();
    }

    /**
     *  Callback called when connection is closed
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(React::AMQP::Connection *connection) override
    {
        // reset connection
        _connection = nullptr;

        // it is not necessary as the connection was the only item in the 
        // event loop, so the loop will stop anyway
    }

    /**
     *  Create the AMQP connection
     *  @return bool
     */
    bool connect()
    {
        // not necessary if already connected
        if (_connection) return true;

        // create the connection
        _connection.reset(new React::AMQP::Connection(&_loop, this, std::string(_json.c_str("host")), 5672,
                                                      ::AMQP::Login(std::string(_json.c_str("user")), std::string(_json.c_str("password"))),
                                                      std::string(_json.c_str("vhost"))));

        // go run the event loop until the connection is connected
        _loop.run();

        // check if the connection still exists (connection will be reset by the
        // onError() function if the connection ran into a problem)
        if (_connection) return true;

        // report error to PHP space
        Php::warning << _error << std::endl;

        // done
        return true;
    }

public:
    /**
     *  Constructor
     *
     *  Watch out: throws an exception when connection could not be established
     *
     *  @param  host
     *  @param  user
     *  @param  password
     *  @param  vhost
     *  @param  exchange
     *  @param  routingkey
     *
     *  @throws std::runtime_error
     */
    Core(const std::string &host, const std::string &user, const std::string &password, const std::string &vhost, std::string &exchange, std::string &routingkey) :
        _connection(new React::AMQP::Connection(&_loop, this, host, 5672, ::AMQP::Login(user, password), vhost))
    {
        // store all properties in the JSON
        _json.set("host", host);
        _json.set("user", user);
        _json.set("password", password);
        _json.set("vhost", vhost);
        _json.set("exchange", exchange);
        _json.set("routingkey", routingkey);

        // go run the event loop until the connection is connected
        _loop.run();

        // if the connection is set back to null, it means that the connection failed,
        // otherwise the connection is still in a valid state
        if (_connection) return;

        // connection was reset, this means that the onError() method was called
        throw std::runtime_error(_error);
    }

    /**
     *  Constructor based on serialized JSON data
     *  @param  data
     */
    Core(const JSON::Object &object) :
        _json(object), _connection(nullptr) {}

    /**
     *  Destructor
     */
    virtual ~Core()
    {
        // nothing to do for us when we're not connected
        if (!_connection) return;

        // close the connection
        _connection->close();

        // wait for the event loop to finish
        _loop.run();
    };

    /**
     *  Method to publish a JSON encoded message to the queue
     *  @param  json
     *  @return bool
     */
    bool publish(const JSON::Object &json)
    {
        // create the connection to the RabbitMQ server
        if (!connect()) return false;

        // create temporary channel, so that possible errors do not affect the connection
        React::AMQP::Channel channel(_connection.get());

        // publish the json
        channel.publish(_json.c_str("exchange"), _json.c_str("routingkey"), json.toString());

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
        if (!_connection) return;

        // step through the loop as long as we have active channels
        // we're working in such a way that all the channels drop away the second
        // they're no longer needed, meaning that we've pushed/retrieved everything
        // from rabbitmq the second we have no channels left
        while (_connection->channels() > 0) _loop.step();
    }

    /**
     *  Run the event loop
     */
    void run()
    {
        // pass on to the loop
        _loop.run();
    }

    /**
     *  Stop the event loop
     */
    void stop()
    {
        // pass on
        _loop.stop();
    }

    /**
     *  Expose the connection 
     *  Returns nullptr on error
     */
    React::AMQP::Connection *connection()
    {
        // connect object
        if (!connect()) return nullptr;

        // retrieve the connection
        return _connection.get();
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
