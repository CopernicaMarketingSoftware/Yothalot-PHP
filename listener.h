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
    std::string _name;


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
        
        // @todo create a new socket
        int newsocket = 0 ; //socket(...);
        
        // @todo accept the incoming connection
        // if (accept(newsocket, _fd) != 0) return;
        
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
        
        // @todo bind the socket
        // @todo listen to the socket
        
        
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
     *  Name of the feedback channel
     *  @todo this should be changed into an "address" property
     */
    virtual const std::string &name() const override
    {
        return _name;
    }
};

