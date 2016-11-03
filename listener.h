/**
 *  Listener.h
 *
 *  Class that listens for incoming connections
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @author David van Erkelens <david.vanerkelens@copernica.com>
 *
 *  @copyright 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <copernica/dns.h>

/**
 *  Class definition
 */
class Listener : public Feedback, private TcpHandler
{
private:
    /**
     *  The socket file descriptor
     *  @var int
     */
    int _fd;
    
    /**
     *  All file descriptors
     *  @var Descriptors
     */
    Descriptors _descriptors;

    /**
     *  The name
     *  @var std::string
     * 
     *  @todo remove this
     */
    mutable std::string _name;

    /**
     *  The IP address we're listening on
     *  @var Copernica::Dns::IpAddress
     */
    mutable Copernica::Dns::IpAddress _ip;

    /**
     *  The port we're listening on
     *  @var int
     */
    mutable int _port = 0;

    /**
     *  The file descriptors that are monitored by this handler
     *  @return Descriptors
     */
    virtual const Descriptors &descriptors() const override
    {
        // expose the member
        return _descriptors;
    }

    /**
     *  Method that is called when a filedescriptor becomes active
     *  @param  fd      the filedescriptor that is active
     *  @param  flags   type of activity (readable or writalble)
     */
    virtual void process(int fd, int flags) override
    {
        // ignore if this is not even our socket
        if (fd != _fd) return;
        
        // store data about the connecting client
        struct sockaddr_in client_address;
        
        // store the struct length
        unsigned int addr_length = sizeof(client_address);
        
        // create a new socket
        int newsocket = accept(_fd, (struct sockaddr *)&client_address, &addr_length);
        
        // was socket creation succesful?
        if (newsocket < 0) return;
        
        // the result buffer
        std::string buffer;
        
        // we are going to read all the way to end of file
        while (true)
        {
            // construct a temporary buffer
            char tempbuf[4096];
            
            // read data into the buffer
            auto bytes = read(newsocket, tempbuf, 4096);
            
            // are we done?
            if (bytes <= 0) break;
            
            // append to the full buffer
            buffer.append(tempbuf, bytes);
        }
        
        // new socket can be closed again
        close(newsocket);
        
        // notify our owner
        _owner->onReceived(this, buffer.data(), buffer.size());
        
        // we're ready!
        _ready = true;
    }

public:
    /**
     *  Constructor
     *  @param  owner
     */
    Listener(Feedback::Owner *owner) : 
        Feedback(owner),
        _fd(socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0))
    {
        // was the socket created?
        if (_fd < 0) throw std::runtime_error("failed to open socket");
        
        // set address settings
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;        
        
        // bind the socket to the file descriptor
        if (bind(_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            // something went wrong...
            throw std::runtime_error("failed to bind socket");
        }
        
        // listen to the socket
        listen(_fd, 5);
        
        // add the file descriptors
        _descriptors.add(_fd, AMQP::readable);
    }
    
    /**
     *  Destructor
     */
    virtual ~Listener()
    {
        // close the socket
        close(_fd);
    }

    /**
     *  Start consuming data from the temporary queue
     */
    virtual void wait() override
    {
        // if the object is ready yet, we do not have to wait
        if (_ready) return;
        
        // because the socket is not marked as blocking, we can just pretend
        // that it is readable -- this will block until the answer comes in
        process(_fd, AMQP::readable);
    }
    
    /**
     *  The tcp channel that is handling incoming results
     *  @return TcpHandler
     */
    virtual TcpHandler *handler() override
    {
        // we are our own handler
        return this;
    }

    /**
     *  Return the IP address we're listening on
     *  @return Copernica::Dns::IpAddress
     */
    const Copernica::Dns::IpAddress &ip() const
    {
        // is the ip already set?
        if (_ip != Copernica::Dns::IpAddress("0.0.0.0")) return _ip;

        // create a socket
        int s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
        
        // was the socket creation succesful?
        if (s >= 0)
        {
            // address of a google DNS server
            Copernica::Dns::IpAddress google("8.8.8.8");
            
            // structure to initialize
            struct sockaddr_in address;
            
            // fill the members
            address.sin_family = AF_INET;
            address.sin_port = htons(53);
            
            // copy address
            memcpy(&address.sin_addr, (const struct in_addr *)google, sizeof(struct in_addr));

            // connect to this address
            if (connect(s, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) == 0)
            {
                // connection succeeded, find out ip
                struct sockaddr_in address;
                socklen_t size = sizeof(address);
                
                // get sock name
                if (getsockname(s, (struct sockaddr *)&address, &size) == 0)
                {
                    // fetch ip address
                    _ip = Copernica::Dns::IpAddress(address.sin_addr);
                }
            }

            // close socket
            close(s);
        }

        // return 
        return _ip;
    }

    /**
     *  Fetch the port we're listening on
     *  @return int
     */
    int port() const
    {
        // is the port already set?
        if (_port > 0) return _port;
        
        // fetch the port we're listening on
        struct sockaddr_in address;
        socklen_t len = sizeof(address);
        getsockname(_fd, (struct sockaddr *)&address, &len);

        // save port
        _port = (int)ntohs(address.sin_port);

        // return
        return _port;
    }
    
    /**
     *  Name of the feedback channel
     *  @todo this should be changed into an "address" property
     */
    virtual const std::string &name() const override
    {
        // did we already construct the name?
        if (_name.length() > 0) return _name;
        
        // we don't have the name yet, construct it
        _name = ip().str() + ":" + std::to_string(port());
        
        // return name
        return _name;
    }
};

