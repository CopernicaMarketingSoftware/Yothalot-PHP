/**
 *  TempQueue.h
 *
 *  Class that creates a temporary queue that can be used for collecting
 *  the results from a Map/Reduce job
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <reactcpp.h>
#include <reactcpp/amqp.h>

/**
 *  Class definition
 */
class TempQueue
{
private:
    /**
     *  Core connection object
     *  @var std::shared_ptr<Core>
     */
    std::shared_ptr<Core> _core;

    /**
     *  Name of the queue
     *  @var std::string
     */
    std::string _name;

    /**
     *  String that is used by the "consume()" call
     *  @var std::string
     */
    std::string _result;

public:
    /**
     *  Constructor
     *
     *  Watch out! An exception is thrown when no RabbitMQ connection is available
     *
     *  @param  core        The core RabbitMQ connection
     *
     *  @throws std::runtime_error
     */
    TempQueue(const std::shared_ptr<Core> &core) : _core(core)
    {
        // we need a connection
        if (!core->connection()) throw std::runtime_error("Not connected to RabbitMQ");

        // we create a temporary channel for creating the queue
        React::AMQP::Channel channel(core->connection());

        // flags for creating the queue
        auto flags = ::AMQP::autodelete|::AMQP::exclusive;

        // declare the queue
        channel.declareQueue(flags).onSuccess([this, core](const std::string &name, uint32_t messagecount, uint32_t consumercount) {

            // assign the queue name
            _name = name;

        }).onFinalize([core]() {

            // stop the loop here so we can return from our start function
            core->stop();
        });

        // run the event loop, until the temporary queue is created (this
        // will call the onFinalized callback, in which we stop the event loop)
        core->run();
    }

    /**
     *  Destructor
     */
    virtual ~TempQueue() 
    {
        // don't do a thing when not connected
        if (!_core->connection()) return;

        // special channel object needed for removing
        React::AMQP::Channel channel(_core->connection());

        // remove the queue from RabbitMQ
        channel.removeQueue(_name);
    }

    /**
     *  Retrieve the name of the temp queue
     *  @return std::string
     */
    const std::string &name() const
    {
        return _name;
    }

    /**
     *  Consume data from the temporary queue
     *  @return std::string
     */
    const std::string &consume()
    {
        // return empty string when not connected
        if (!_core->connection()) return _result;

        // we create a special channel to start the consumer
        auto channel = std::make_shared<React::AMQP::Channel>(_core->connection());

        // empty result for now
        _result.clear();

        // start consuming from our temporary queue
        channel->consume(_name).onReceived([this, channel](const ::AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {

            // assign the response
            _result.assign(message.body(), message.bodySize());

            // ack the message
            channel->ack(deliveryTag);

            // stop further consuming
            channel->cancel(_name);

            // stop the event loop
            _core->stop();

        }).onError([this](const char *message) {

            // the consumer failed, we stop the event loop to prevent endless loop
            _core->stop();
        });

        // run the event loop until the consumer is ready
        _core->run();

        // done
        return _result;
    }
};
