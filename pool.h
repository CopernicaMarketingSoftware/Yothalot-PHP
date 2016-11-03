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
class Pool : public Php::Base, public Php::Countable
{
private:
    /**
     *  All jobs that are being monitored by this pool
     *  @var std::map
     */
    std::map<Job*,Php::Value> _jobs;

    /**
     *  Set of all TCP handles via which data can come one
     *  @var std::set
     */
    std::set<TcpHandler*> _handlers;
    
    
    /**
     *  Called when a filedescriptor becomes active
     *  @param  fd      the active filedescriptor
     *  @param  flags   readability/writabilitie flags
     */
    void process(int fd, int flags) const
    {
        // check all tcp handlers
        for (const auto &handler : _handlers) handler->process(fd, flags);
    }

    /**
     *  Get a job that is ready
     *  @return Php::Value
     */
    Php::Value extract()
    {
        // iterate over the jobs
        for (auto iter : _jobs)
        {
            // get access to the job
            Job *job = iter.first;
            
            // move on if the job is not ready
            if (!job->ready()) continue;
            
            // this job is ready, store the php variable
            Php::Value result = iter.second;
            
            // remove it from the map
            _jobs.erase(job);
            
            // done
            return result;
        }
        
        // no job was ready
        return nullptr;
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
        
        // add the tcp handler
        _handlers.insert(wrapper->handler());
    }
    
    /**
     *  Expose a job that is ready
     *  @return Php::Value
     */
    Php::Value fetch()
    {
        // skip if there are no more tcp handlers listed
        if (_handlers.empty()) return nullptr;
        
        // we are going to create one big event loop with the file descriptors of all connections
        Descriptors descriptors;
    
        // check all connections
        for (const auto &handler : _handlers) descriptors.add(handler->descriptors());

        // construct an event loop based on all these file descriptors
        Loop loop(descriptors);
        
        // run the event loop, but do not block
        while (loop.step(std::bind(&Pool::process, this, _1, _2), false)) { /* keep iterating */ }
        
        // extract a job
        return extract();
    }
    
    /**
     *  Size of the job
     *  @return Php::Value
     */
    Php::Value size() const
    {
        return (int)_jobs.size();
    }
    
    /**
     *  Size of the job
     *  @return unsigned
     */
    virtual long count() override
    {
        return _jobs.size();
    }
    
    /**
     *  Wait for the first job that is ready, and return that job
     *  @return Php::Value
     */
    Php::Value wait()
    {
        // skip if there are no more connections listed
        if (_handlers.empty()) return nullptr;
        
        // we are going to create one big event loop with the file descriptors of all connections
        Descriptors descriptors;
    
        // check all tcp handlers
        for (const auto &handler : _handlers) descriptors.add(handler->descriptors());
        
        // construct an event loop based on all these file descriptors
        Loop loop(descriptors);

        // keep looping as long as we have jobs
        while (!_jobs.empty())
        {
            // get a job that is ready
            auto job = extract();
            
            // we're done if we indeed has a ready job
            if (!job.isNull()) return job;
            
            // let's take one more step in the event loop (this is not so efficient,
            // better would be to run the event loop until we're sure that a job is ready)
            loop.step(std::bind(&Pool::process, this, _1, _2));
        }
        
        // there are no more active jobs
        _handlers.clear();
        
        // impossible to return a job
        return nullptr;
    }
};

