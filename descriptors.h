/**
 *  Descriptors.h
 *
 *  Class that collects all filedescriptors that are in use
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>    
 *  @author Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 - 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <set>

/**
 *  Class definition
 */
class Descriptors
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


public:
    /**
     *  Constructor
     */
    Descriptors() = default;

    /**
     *  Destructor
     */
    virtual ~Descriptors() = default;

    /**
     *  Add another set of descriptors
     *  @param  that
     */
    void add(const Descriptors &that)
    {
        // merge all the sets
        _all.insert(that._all.begin(), that._all.end());
        _read.insert(that._read.begin(), that._read.end());
        _write.insert(that._write.begin(), that._write.end());
        
        // check highest
        _highest = std::max(_highest, that._highest);
    }

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
};
