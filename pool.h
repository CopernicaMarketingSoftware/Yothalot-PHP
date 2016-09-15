/**
 *  Pool.h
 *
 *  Class that can group multiple running Yothalot jobs, and that waits
 *  for the first job in the pool that is ready
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  copyright 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Class definition
 */
class Pool : public Php::Base
{
private:
    /**
     *  All jobs that are being monitored by this pool
     *  @var std::map
     */
    std::map<Job*,Php::Value> _jobs;

    /**
     *  Set of all connections
     *  @var std::set
     */
    std::set<std::shared_ptr<Core>> _cores;
    
    
    /**
     *  Called when a filedescriptor becomes active
     *  @param  fd      the active filedescriptor
     *  @param  flags   readability/writabilitie flags
     */
    void process(int fd, int flags) const
    {
        // check all connections
        for (const auto &core : _cores) core->connection()->process(fd, flags);
    }

public:
    /**
     *  Constructor
     */
    Pool() = default;
    
    /**
     *  Destructor
     */
    virtual ~Pool() = default;
    
    /**
     *  Add a job to the pool
     *  @param  params
     */
    void add(Php::Parameters &params)
    {
        // get the job as php variable
        Php::Value phpjob = params[0];
        
        // must be a yothalot job
        if (!phpjob.instanceOf("Yothalot\\Job")) throw Php::Exception("Not a valid job supplied");
        
        // convert to the job wrapper
        auto *wrapper = (Job *)phpjob.implementation();
        
        // make sure the job is started
        wrapper->start();
        
        // add the jobimpl class
        _jobs.insert(std::make_pair(wrapper, phpjob));
        
        // add the core connection
        _cores.insert(wrapper->core());
    }
    
    /**
     *  Wait for the first job that is ready, and return that job
     *  @return Php::Value
     */
    Php::Value wait()
    {
        // we are going to create one big event loop with the file descriptors of all connections
        Descriptors descriptors;
        
        // check all connections
        for (const auto &core : _cores) descriptors.add(core->descriptors());
        
        // construct the event loop
        Loop loop(descriptors);
        
        // run the loop
        loop.run(std::bind(&Pool::process, this, _1, _2));
        
        // @todo better implementation
        return nullptr;
    }
};

