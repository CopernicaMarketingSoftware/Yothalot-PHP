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
     *  Filedescriptors for readability and writability
     *  @var std::set
     */
    std::set<int> _read;
    std::set<int> _write;

    /**
     *  All filedescriptors
     *  @var std::set
     */
    std::set<int> _all;

    /**
     *  Highest descriptor in the set
     *  @var int
     */
    int _highest = 0;

    /**
     *  Is the loop active?
     *  @var bool
     */
    bool _active = false;

public:
    /**
     *  Constructor
     */
    Loop() = default;

    /**
     *  Destructor
     */
    virtual ~Loop() = default;

    /**
     *  Add a filedescriptor
     *  @param  fd
     *  @param  flags
     */
    void add(int fd, int flags)
    {
        // should we remove instead?
        if (flags == 0) return remove(fd);

        // add to all set
        _all.insert(fd);

        // add to appropriate sets
        if (flags & AMQP::readable) _read.insert(fd);
        if (flags & AMQP::writable) _write.insert(fd);

        // was this the highest?
        if (fd > _highest) _highest = fd;
    }

    /**
     *  Remove a filedescriptor
     *  @param  fd
     */
    void remove(int fd)
    {
        // remove from all sets
        _all.erase(fd);
        _read.erase(fd);
        _write.erase(fd);

        // was this the highest?
        if (fd != _highest) return;

        // we need to find out the new highest number
        _highest = _all.empty() ? 0 : *_all.rbegin();
    }

    /**
     *  Do a single loop step
     *  @param  connection      The connection
     *  @return bool            True on success - false when there is nothing to step
     */
    bool step(AMQP::TcpConnection *connection)
    {
        // is there something to check?
        if (_all.empty()) return false;

        // create two sets
        FdSet readable(_read);
        FdSet writable(_write);

        // wait for the sets
        auto result = select(_highest + 1, readable, writable, nullptr, nullptr);

        // on signal errors we still return true because the event loop is still valid,
        // and calling step() again is meaningful
        if (result < 0 || errno == EINTR) return true;

        // big problems on all other errors, or when no files are active (which is 
        // completely impossible because we have no timer, so by now a file MUST be active)
        if (result <= 0) return false;

        // check which filedescriptors is readable
        for (auto fd : _all)
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
     *  Stop running the loop
     */
    void stop()
    {
        // no longer active
        _active = false;
    }
};