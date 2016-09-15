/**
 *  Pool.h
 *
 *  Class that can group multiple running Yothalot jobs, and that waits
 *  for the first job in the pool that is ready
 *
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
     *  All the AMQP connections used by all the jobs
     *  @var std::vector
     */
    std::vector<std::shared_ptr<Core>> _connections;

    /**
     *  All jobs that are being monitored by this pool
     *  @var std::vector
     */
    std::vector<Php::Value> _jobs;


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
        
        // @todo prevent doubles
        
        // add to the vector
        _jobs.push_back(phpjob);
    }
    
    /**
     *  Wait for the first job that is ready, and return that job
     *  @return Php::Value
     */
    Php::Value wait()
    {
        // not yet implementation
        // @todo add implementation
        return nullptr;
    }
};

