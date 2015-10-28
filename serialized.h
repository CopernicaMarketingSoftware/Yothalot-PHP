/**
 *  Serialized.h
 *
 *  Object that takes care of serializing and unserializing a job
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "json/object.h"
#include "jobimpl.h"

/**
 *  Class definition
 */
class Serialized : public JSON::Object
{
public:
    /**
     *  Constructor to serialize
     *
     *  Watch out! Object throws an error when the job is not serializable (this
     *  happens when it has no directory on GlusterFS)
     *
     *  @param  impl        Job implementation object that is going to be serialized
     *
     *  @throws std::runtime_error
     */
    Serialized(JobImpl *impl)
    {
        // does this job have a directory? otherwise serializing does not work
        if (!impl->directory()) throw std::runtime_error("Only jobs that are started with access to a GlusterFS mount point can be serialized");

        // one member holds all job data
        set("job", impl->json());

        // and the other member holds all connection data
        set("connection", impl->core()->json());
    }

    /**
     *  Constructor to unserialize
     *  @param  buffer
     *  @param  size
     */
    Serialized(const char *buffer, size_t size) : 
        JSON::Object(buffer, size) {}

    /**
     *  Destructor
     */
    virtual ~Serialized() {}
};

