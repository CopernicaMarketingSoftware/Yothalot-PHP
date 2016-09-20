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
public:
    /**
     *  Interface implemented by a queue-owner
     */
    class Owner
    {
    public:
        /**
         *  Called when result comes in
         *  @param  queue
         *  @param  buffer
         *  @param  size
         */
        virtual void onReceived(TempQueue *queue, const char *buffer, size_t size) = 0;
        
        /**
         *  Called in case of an error
         *  @param  queue
         *  @param  message
         */
        virtual void onError(TempQueue *queue, const char *message) = 0;
    };
    
private:
    /**
     *  Pointer to the owner
     *  @var Owner
     */
    Owner *_owner;

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
     *  Was the consumer cancelled?
     *  @var bool
     */
    bool _cancelled = false;

    /**
     *  Is the object ready, or do we still have to clean up things?
     *  @var bool
     */
    bool _ready = false;

    
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
        
        // we stop the private event loop, we're going to restart it when the owner starts consuming
        _loop.stop();
    }
    
    /**
     *  Method that is called when the message has been consumed
     *  @param  message
     *  @param  deliverytag
     */
    void onReceived(const AMQP::Message &message, uint64_t deliveryTag)
    {
        // ack the message
        _channel.ack(deliveryTag);

        // stop further consuming
        _channel.cancel(_name).onSuccess(std::bind(&TempQueue::onCancelled, this, _1));

        // tell the owner
        _owner->onReceived(this, message.body(), message.bodySize());
    }
    
    /**
     *  Channel errors
     *  @param  message
     */
    void onError(const char *message)
    {
        // remember that we're ready, nothing left to clean up
        _ready = true;
        
        // stop event loop
        _loop.stop();
        
        // pass to the owner
        _owner->onError(this, message);
    }
    
    /**
     *  Consuming was cancelled
     *  @param  consumer
     */
    void onCancelled(const std::string &consumer)
    {
        // remember that consumer has been cancelled
        _cancelled = true;
        
        // we no longer need the queue
        _channel.removeQueue(_name).onSuccess(std::bind(&TempQueue::onRemoved, this, _1));
    }
    
    /**
     *  Callback when queue was removed
     *  @param  messages
     */
    void onRemoved(size_t messages)
    {
        // close the channel
        _channel.close().onSuccess(std::bind(&TempQueue::onClosed, this));
    }
    
    /**
     *  Callback when channel was closed
     */
    void onClosed()
    {
        // object is ready
        _ready = true;

        // stop private event loop
        _loop.stop();
    }
    
public:
    /**
     *  Constructor
     *  @param  owner       Object that will be notified with the result
     *  @param  core        The core RabbitMQ connection
     */
    TempQueue(Owner *owner, const std::shared_ptr<Core> &core) : 
        _owner(owner), _core(core), _loop(core->descriptors()), _channel(core->connection())
    {
        // set up error handler
        _channel.onError(std::bind(&TempQueue::onError, this, _1));
        
        // flags for creating the queue
        auto flags = ::AMQP::autodelete | ::AMQP::exclusive;

        // declare the queue
        _channel.declareQueue(flags).onSuccess(std::bind(&TempQueue::onDeclared, this, _1));
        
        // run the event loop, because we need to know the name, this will run
        // until the declareQueue operation is finished
        _loop.run(core->connection());
    }

    /**
     *  Destructor
     */
    virtual ~TempQueue()
    {
        // if we still have things to clean up, we're going to run the event loop a little longer
        if (_ready) return;
        
        // cancel the consumer
        if (!_cancelled) _channel.cancel(_name).onSuccess(std::bind(&TempQueue::onCancelled, this, _1));
        
        // run the event loop a little longer
        _loop.run(_core->connection());
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
     *  Start consuming data from the temporary queue
     */
    void wait()
    {
        // if object is ready, we do not have to do anything
        if (_ready) return;
        
        // start the event loop, it will come to an end when we have the result
        _loop.run(_core->connection());
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
