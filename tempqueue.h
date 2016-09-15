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
#include <amqpcpp.h>

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
     *  The event loop
     *  @var Loop
     */
    Loop _loop;
    
    /**
     *  AMQP channel that will be used for consuming
     *  @var AMQP::TcpChannel
     *
     *  @todo the connection->channels() > 0 - do we still use that?
     */
    AMQP::TcpChannel _channel;

    /**
     *  Name of the queue
     *  @var std::string
     */
    std::string _name;

    /**
     *  Is the result available
     *  @return bool
     */
    bool _ready = false;

    /**
     *  String that is used by the "consume()" call
     *  @var std::string
     */
    std::string _result;
    
    
    /**
     *  Method that is called when the queue has been declared
     *  @param  name
     */
    void onDeclared(const std::string &name)
    {
        // store the name
        _name = name;
        
        // start the consumer
        _channel.consume(_name).onReceived(std::bind(&TempQueue::onReceived, this, _1, _2));
    }
    
    /**
     *  Method that is called when the message has been consumed
     *  @param  message
     *  @param  deliverytag
     */
    void onReceived(const AMQP::Message &message, uint64_t deliveryTag)
    {
        // object is ready
        _ready = true;
        
        // assign the response
        _result.assign(message.body(), message.bodySize());

        // ack the message
        _channel.ack(deliveryTag);

        // stop further consuming
        _channel.cancel(_name).onFinalize(std::bind(&TempQueue::onFinished, this));
    }
    
    /**
     *  Channel errors
     *  @param  message
     */
    void onError(const char *message)
    {
        // stop event loop
        _loop.stop();
    }
    
    /**
     *  An operation is finished
     */
    void onFinished()
    {
        // stop event loop
        _loop.stop();
    }
    
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
    TempQueue(const std::shared_ptr<Core> &core) : _core(core), _loop(core->descriptors()), _channel(core->connection())
    {
        // set up error handler
        _channel.onError(std::bind(&TempQueue::onError, this, _1));
        
        // flags for creating the queue
        auto flags = ::AMQP::autodelete | ::AMQP::exclusive;

        // declare the queue
        _channel.declareQueue(flags).onSuccess(std::bind(&TempQueue::onDeclared, this, _1)).onFinalize(std::bind(&TempQueue::onFinished, this));
        
        // run the event loop, because we need to know the name, this will run
        // until the declareQueue operation is finished
        _loop.run(core->connection());
    }

    /**
     *  Destructor
     */
    virtual ~TempQueue()
    {
        // remove the queue from RabbitMQ
        _channel.removeQueue(_name);
    }

    /**
     *  Retrieve the name of the temp queue
     *  @return std::string
     */
    const std::string &name() const
    {
        // expose member
        return _name;
    }

    /**
     *  Consume data from the temporary queue
     *  @return std::string
     */
    const std::string &consume()
    {
        // do we already have a result?
        if (_ready) return _result;
        
        // start the event loop, it will come to an end when we have the result
        _loop.run(_core->connection());
        
        // we expect to have the result
        return _result;
    }
    
    /**
     *  Is the result already available
     *  @return bool
     */
    bool ready() const
    {
        // this is stored in a member
        return _ready;
    }
};
