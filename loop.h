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
     *  Destructor
     */
    virtual ~Loop() = default;

    /**
     *  Do a single loop step
     *  @param  connection      The connection
     *  @return bool            True on success - false when there is nothing to step
     */
    bool step(AMQP::TcpConnection *connection)
    {
        // is there something to check?
        if (!_descriptors) return false;
        
        // the readable and writable sets
        FdSet readable(_descriptors.readable());
        FdSet writable(_descriptors.writable());

        // wait for the sets
        auto result = select(_descriptors.highest() + 1, readable, writable, nullptr, nullptr);

        // on signal errors we still return true because the event loop is still valid,
        // and calling step() again is meaningful
        if (result < 0 || errno == EINTR) return true;

        // big problems on all other errors, or when no files are active (which is 
        // completely impossible because we have no timer, so by now a file MUST be active)
        if (result <= 0) return false;

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

            // notify the connection
            connection->process(fd, flags);

            // jump out of loop - connection might no longer exist
            break;
        }

        // we are ready
        return true;
    }

    /**
     *  Run the event loop to the end
     *  @param  connection      The AMQP connection being monitored
     *  @return int
     */
    int run(AMQP::TcpConnection *connection)
    {
        // the user wants to run the loop - this means we're active
        _active = true;

        // keep looping while the loop is active (inside the connection, the loop
        // could be stopped, by calling Loop::stop(), so that we can jump out of
        // this main loop
        while (_active)
        {
            // take one step
            if (!step(connection)) break;
        }

        // loop is no longer active
        _active = false;

        // done
        return 0;
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

