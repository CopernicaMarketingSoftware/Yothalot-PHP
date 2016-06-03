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
     *  A thread loop that the actual connection will run on
     *  @var  Loop
     */
    Loop _loop;

    /**
     *  The underlying TCP connection
     *  @var  AMQP::TcpConnection
     */
    std::unique_ptr<AMQP::TcpConnection> _connection;

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
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override
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
    virtual void onConnected(AMQP::TcpConnection *connection) override
    {
        // stop the loop
        _loop.stop();
    }

    /**
     *  Callback called when connection is closed
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(AMQP::TcpConnection *connection) override
    {
        // reset connection
        _connection = nullptr;
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
        _loop.add(fd, flags);
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
        _connection.reset(new AMQP::TcpConnection(this, AMQP::Address(std::string(_json.c_str("host")), 5672,
                                                  ::AMQP::Login(std::string(_json.c_str("user")), std::string(_json.c_str("password"))),
                                                  std::string(_json.c_str("vhost")))));

        // check if the connection still exists (connection will be reset by the
        // onError() function if the connection ran into a problem)
        if (_connection) return true;

        // report error to PHP space
        if (!_error.empty()) Php::warning << _error << std::endl;

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
     *  @param  mapreduce
     *  @param  races
     *  @param  jobs
     *
     *  @throws std::runtime_error
     */
    Core(const std::string &host, const std::string &user, const std::string &password, const std::string &vhost, const std::string &exchange, const std::string &mapreduce, const std::string &races, const std::string &jobs) :
        _connection(new AMQP::TcpConnection(this, AMQP::Address(host, 5672, ::AMQP::Login(user, password), vhost)))
    {
        // store all properties in the JSON
        _json.set("host", host);
        _json.set("user", user);
        _json.set("password", password);
        _json.set("vhost", vhost);
        _json.set("exchange", exchange);
        _json.set("mapreduce", mapreduce);
        _json.set("races", races);
        _json.set("jobs", jobs);

        // go run the event loop until the connection is connected
        _loop.run(_connection.get());

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
        while (_connection) _loop.run(_connection.get());
    };

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
        AMQP::TcpChannel channel(_connection.get());

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
        if (!_connection) return;

        // step through the loop as long as we have active channels
        // we're working in such a way that all the channels drop away the second
        // they're no longer needed, meaning that we've pushed/retrieved everything
        // from rabbitmq the second we have no channels left
        while (_connection->channels() > 0) _loop.step(_connection.get());
    }

    /**
     *  Stop the event loop
     */
    void stop()
    {
        // pass on to the loop
        _loop.stop();
    }

    /**
     *  Run the event loop
     */
    void run()
    {
        // pass on to the loop
        _loop.run(_connection.get());
    }

    /**
     *  Expose the connection 
     *  Returns nullptr on error
     */
    AMQP::TcpConnection *connection()
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
