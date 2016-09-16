/**
 *  Loop.h
 *
 *  Simple event loop implementation
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
#include <amqpcpp.h>
#include <set>
#include <sys/select.h>
#include "fdset.h"

/**
 *  Class definition
 */
class Loop
{
private:
    /**
     *  Reference to the filedescriptors
     *  @var Descriptors
     */
    const Descriptors &_descriptors;

    /**
     *  Is the loop active?
     *  @var bool
     */
    bool _active = false;

public:
    /**
     *  Constructor
     *  @param  descriptors
     */
    Loop(const Descriptors &descriptors) : _descriptors(descriptors) {}

    /**
     *  Copy constructor
     *  @param  that
     */
    Loop(const Loop &that) = delete;

    /**
     *  Destructor
     */
    virtual ~Loop() = default;

    /**
     *  Do a single loop step
     *  @param  callback        Callback method
     *  @param  block           Is it ok to block?
     *  @return bool            Was there any activity?
     */
    int step(const std::function<void(int,int)> &callback, bool block = true)
    {
        // is there something to check?
        if (!_descriptors) return false;
        
        // the readable and writable sets
        FdSet readable(_descriptors.readable());
        FdSet writable(_descriptors.writable());
        
        // construct a timeout
        struct timeval timeout;
        
        // timeout of zero seconds
        memset(&timeout, 0, sizeof(struct timeval));

        // wait for the sets
        auto result = select(_descriptors.highest() + 1, readable, writable, nullptr, block ? nullptr : &timeout);

        // on signal errors we still return true because the event loop is still valid,
        // and calling step() again is meaningful
        if (result < 0 && errno == EINTR) return true;
        
        // if no files are active, things are also valid
        if (result == 0) return false;

        // big problems on all other errors
        if (result < 0) return false;

        // check which filedescriptors is readable
        for (auto fd : _descriptors)
        {
            // the readable + writable flags
            int flags = 0;

            // is this filedescriptor readable or writable
            if (readable.contains(fd)) flags |= AMQP::readable;
            if (writable.contains(fd)) flags |= AMQP::writable;

            // skip if not active
            if (flags == 0) continue;

            // notify the callback
            callback(fd, flags);

            // jump out of loop - connection might no longer exist
            break;
        }

        // we are ready, and there was activity
        return true;
    }

    /**
     *  Run the event loop to the end
     *  @param  callback        Callback method
     */
    void run(const std::function<void(int,int)> &callback)
    {
        // the user wants to run the loop - this means we're active
        _active = true;

        // keep looping while the loop is active (inside the connection, the loop
        // could be stopped, by calling Loop::stop(), so that we can jump out of
        // this main loop
        while (_active)
        {
            // take one step
            if (!step(callback, true)) break;
        }

        // loop is no longer active
        _active = false;
    }

    /**
     *  Do a single loop step
     *  @param  connection      The connection
     *  @return bool            True if there was activity
     */
    bool step(AMQP::TcpConnection *connection)
    {
        // pass on
        return step(std::bind(&AMQP::TcpConnection::process, connection, _1, _2));
    }

    /**
     *  Run the event loop
     *  @param  connection      The connection
     */
    void run(AMQP::TcpConnection *connection)
    {
        // pass on
        run(std::bind(&AMQP::TcpConnection::process, connection, _1, _2));
    }
    
    /**
     *  Stop the event loop, this can be called if you started the loop with run to stop it
     */
    void stop()
    {
        // reset the boolean
        _active = false;
    }
};

