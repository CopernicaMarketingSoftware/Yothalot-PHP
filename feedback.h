/**
 *  Feedback.h
 *
 *  Interface that is implemented by all the possible feedback channels
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2016 Copernica BV
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
class Feedback
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
        virtual void onReceived(Feedback *queue, const char *buffer, size_t size) = 0;
        
        /**
         *  Called in case of an error
         *  @param  queue
         *  @param  message
         */
        virtual void onError(Feedback *queue, const char *message) = 0;
    };
    
protected:
    /**
     *  Pointer to the owner
     *  @var Owner
     */
    Owner *_owner;

    /**
     *  Is the object ready
     *  @var bool
     */
    bool _ready = false;

    /**
     *  Protected constructor
     *  @param  owner       Object that will be notified with the result
     */
    Feedback(Owner *owner) : _owner(owner) {}

public:
    /**
     *  Destructor
     */
    virtual ~Feedback() = default;

    /**
     *  Start consuming data from the temporary queue
     */
    virtual void wait() = 0;
    
    /**
     *  The tcp channel that is handling incoming results
     *  @return TcpHandler
     */
    virtual TcpHandler *handler() = 0;
    
    /**
     *  Name of the feedback channel
     *  @todo this should be changed into an "address" property
     */
    virtual const std::string &name() const = 0;
    
    /**
     *  Is the result already available?
     *  @return bool
     */
    bool ready() const
    {
        // this is stored in a member
        return _ready;
    }
};
