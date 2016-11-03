/**
 *  TcpHandler.h
 *
 *  Interface that is implemented by classes that are monitoring one
 *  or more sockets, and that want to be notified when such a socket
 *  becomes active.
 *
 *  This is implemented by RabbitMQ connections and TCP sockets that
 *  wait for the result of a job
 *
 *  @copyright 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Class definition
 */
class TcpHandler
{
public:
    /**
     *  The file descriptors that are monitored by this handler
     *  @return Descriptors
     */
    virtual const Descriptors &descriptors() const = 0;

    /**
     *  Method that is called when a filedescriptor becomes active
     *  @param  fd      the filedescriptor that is active
     *  @param  flags   type of activity (readable or writalble)
     */
    virtual void process(int fd, int flags) = 0;
};
