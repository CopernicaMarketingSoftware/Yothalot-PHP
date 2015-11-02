/**
 *  FdSet.h
 *
 *  Simple wrapper class around a fd-set
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
#include <set>
#include <sys/select.h>

/**
 *  Class definition
 */
class FdSet
{
private:
    /**
     *  The fd-set
     *  @var fd_set
     */
    fd_set _set;

public:
    /**
     *  Constructor
     *  @param  fds
     */
    FdSet(const std::set<int> &fds)
    {
        // set set to all zero's
        FD_ZERO(&_set);

        // iterate over the fds
        for (auto fd : fds) FD_SET(fd, &_set);
    }

    /**
     *  Destructor
     */
    virtual ~FdSet() = default;

    /**
     *  Cast to fd-set
     *  @return fd_set*
     */
    operator fd_set* () 
    {
        // expose member
        return &_set;
    }

    /**
     *  Check if a filedescriptor is included
     *  @param  fd
     *  @return bool
     */
    bool contains(int fd) const
    {
        // check
        return FD_ISSET(fd, &_set);
    }
};