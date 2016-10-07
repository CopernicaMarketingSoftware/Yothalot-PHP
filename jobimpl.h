/**
 *  JobImpl.h
 *
 *  Because the PHP constructor runs _after_ the C++ constructor,
 *  we store all data that should not directly be constructed in
 *  a nested helper class, which is constructed in the __construct()
 *  or unserialize() methods
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <limits.h>
#include "data.h"
#include "tempqueue.h"
#include "wrapper.h"

/**
 *  Class definition
 */
class JobImpl : private TempQueue::Owner
{
private:
    /**
     *  All JSON data for the job
     *  @var Data
     */
    Data _json;

    /**
     *  Shared pointer to the core AMQP connection
     *  @var std::shared_ptr<Core>
     */
    std::shared_ptr<Core> _core;

    /**
     *  Did we create any input yet?
     *  @var bool
     */
    bool _input = false;

    /**
     *  Did we start yet?
     *  @var bool
     */
    bool _started = false;

    /**
     *  Are we done yet?
     *  @var bool
     */
    bool _done = false;

    /**
     *  Did an error occur?
     *  @var bool
     */
    bool _error = false;

    /**
     *  The temporary queue the result will be published to
     *  @var TempQueue
     */
    std::unique_ptr<TempQueue> _tempqueue;

    /**
     *  The temporary directory holding all input data
     *  @var Directory
     */
    std::unique_ptr<Directory> _directory;

    /**
     *  The output file to which records are written
     *  @var Yothalot::Output
     */
    std::unique_ptr<Yothalot::Output> _output;

    /**
     *  Result data sent back
     *  @var JSON::Object
     */
    JSON::Object _result;

    /**
     *  Split size for the file
     *  @var    size_t
     */
    size_t _splitsize = 10 * 1024 * 1024;

    
    /**
     *  Called when result comes in
     *  @param  queue
     *  @param  buffer
     *  @param  size
     */
    virtual void onReceived(TempQueue *queue, const char *buffer, size_t size) override
    {
        // assign to the result variable
        _result = JSON::Object(buffer, size);

        // we're done
        _done = true;

        // in case case of a task we may have the stderr in the main json right away
        if (isTask() && _result.contains("stderr")) _error = true;

        // if we don't have an error we return true
        else if (_result.contains("error")) _error = true;
        
        // nothing left to be done on error, or when this is not a map-reduce job
        if (_error || !isMapReduce() || _result.object("finalizers").integer("processes") > 0) return;
        
        // name of the directory that contains the result files
        const char *directory = _result.c_str("directory");
        
        // leap out if there is no directory with files
        if (directory == nullptr) return;
        
        // prevent exceptions (the Directory object might throw)
        try
        {
            // hey. the finalizer did not yet run on the yothalot cluster, that means that we
            // have to do the finalizing in this process
            Wrapper mapreduce(_json.finalizer());
            
            // create the write task
            Yothalot::WriteTask task(base(), &mapreduce);
            
            // construct the full directory name
            Directory dir(_result.c_str("directory"));
            
            // traverse over the dir
            dir.traverse([&task, &dir](const char *name) {
                
                // construct absolute path name of the file
                std::string fullname(dir.full());
                
                // add directory name
                fullname.append("/").append(name);
                
                // pass to the task
                task.process(fullname.data(), 0, INT_MAX, 0, INT_MAX);
            });
            
            // remove the directory
            dir.remove();
        }
        catch (...)
        {
            // the result failed
        }
    }
    
    /**
     *  Called in case of an error
     *  @param  queue
     *  @param  message
     */
    virtual void onError(TempQueue *queue, const char *message) override
    {
        // we're done
        _done = true;

        // remember that we're in an error state
        _error = true;
    }

    /**
     *  Get access to the output file
     *  @param  flush       flush the output file / create a new one
     *  @return Yothalot::Output
     */
    Yothalot::Output *output(bool flush = false)
    {
        // do we already have such a file?
        if (_output && !flush) return _output.get();

        // file is not yet opened, but we need a directory on glusterfs for that
        if (!_directory) return nullptr;

        // construct a new output file
        _output.reset(new Yothalot::Output(std::string(_directory->full()) + "/" + Php::call("uniqid").stringValue(), _splitsize));

        // we have no created an input file
        _input = true;

        // done
        return _output.get();
    }

public:
    /**
     *  Constructor for constructing a brand new job
     *  @param  core        The core connection object
     *  @param  algo        User supplied algorithm object
     */
    JobImpl(const std::shared_ptr<Core> &core, const Php::Value &algo) :
        _json(algo),
        _core(core)
    {
        // try to allocate a directory
        try
        {
            // create the directory
            _directory.reset(new Directory());

            // the directory exists, set this in the json, we want the cleanup and no server
            if (_json.isMapReduce()) _json.directory(_directory->relative(), true, nullptr);

            // either a race job or an old mapreduce job; add the directory directly
            else _json.directory(_directory->relative());
        }
        catch (...)
        {
            // failed to allocate the directory, we do not have to
            // deal with this, because the input object already held
            // an array of data
        }
    }

    /**
     *  Constructor for an unserialized job
     *
     *  This throws an exception if the directory could not be read (in
     *  which case it is pointless to unserialize, because then the
     *  job would not be moveable to other servers anyway)
     *
     *  @param  data        A JSON object holding unserialized data
     *
     *  @throws std::runtime_error
     */
    JobImpl(const JSON::Object &data) :
        _json(data.object("job")) // we don't create a connection here on purpose, as we just don't need one
    {
        // does the input json contain a specific directory where we can dump temporary files?
        if (!_json.directory()) return;

        // input json contains a directory, construct this
        _directory.reset(new Directory(_json.directory()));

        // @todo revive algorithm object, and use that for finalizing
    }

    /**
     *  Destructor
     */
    virtual ~JobImpl() = default;

    /**
     *  Simple checkers for race and mapreduce
     *  @return bool
     */
    bool isRace() const { return _json.isRace(); }
    bool isMapReduce() const { return _json.isMapReduce(); }
    bool isTask() const { return _json.isTask(); }

    /**
     *  Get the algorithm used by the job
     *
     *  @return The used algorithm
     */
    Algorithm algorithm() const
    {
        // just retrieve the algorithm from the data
        return _json.algorithm();
    }

    /**
     *  Set the split-size to be used for input used
     *  in the mapper task.
     *
     *  @param  splitsize   The desired split size for created input files
     *  @return Was the split size successfully changed (this can only be done before input is generated)
     */
    bool splitsize(size_t splitsize)
    {
        // not possible if we have already generated input
        if (_input) return false;

        // update the split size
        _splitsize = splitsize;

        // success!
        return true;
    }

    /**
     *  Relative path name of the temporary directory
     *  @return const char
     */
    const char *directory() const
    {
        // check if dir is set, then return the relative path
        return _directory ? _directory->relative() : nullptr;
    }

    /**
     *  Setters for max processes
     *  @param  value
     *  @return bool
     */
    bool maxprocesses(int value)
    {
        // not possible if job was already started
        if (_started) return false;

        // set in the JSON
        _json.maxprocesses(value);

        // done
        return true;
    }

    /**
     *  setter for max number of mappers
     *  @param  value
     *  @return bool
     */
    bool maxmappers(int value)
    {
        // not possible if job was already started
        if (_started) return false;

        // set in the JSON
        _json.maxmappers(value);

        // done
        return true;
    }

    /**
     *  Setter  for max number of reducer processes
     *  @param value
     *  @return bool
     */
    bool maxreducers(int value)
    {
        // not possible if job sas already started
        if (_started) return false;

        // set in the JSON
        _json.maxreducers(value);

        // done
        return true;
    }

    /**
     *  Setter  for max number of finalizer processes
     *  @param value
     *  @return bool
     */
    bool maxfinalizers(int value)
    {
        // not possible if job has already started
        if (_started) return false;

        // set in the JSON
        _json.maxfinalizers(value);

        // done
        return true;
    }

    /**
     *  setter for the modulo property
     *  @param value
     *  @return bool
     */
    bool modulo(int value)
    {
        // not possible if job has already started
        if (_started) return false;

        // set in the json
        _json.modulo(value);

        // done
        return true;
    }

    /**
     *  setter for the max number of files
     *  @param value
     *  @return bool
     */
    bool maxfiles(int mapper, int reducer, int finalizer)
    {
        // not possible if job has already started
        if (_started) return false;

        // set in the json
        _json.maxfiles(mapper, reducer, finalizer);

        // done
        return true;
    }

    /**
     *  setter for the max number of bytes
     *  @param value
     *  @return bool
     */
    bool maxbytes(int64_t mapper, int64_t reducer, int64_t finalizer)
    {
        // not possible if job has already started
        if (_started) return false;

        // the byte limit for each mapper *must* be a multiple of the
        // split size of the generated input files, because they are
        // compressed, so we can only read from the start of a split
        if (mapper    % _splitsize) return false;
        if (reducer   % _splitsize) return false;
        if (finalizer % _splitsize) return false;

        // set in the json
        _json.maxbytes(mapper, reducer, finalizer);

        // done
        return true;
    }

    /**
     *  Setter for the maximum number of records per mapper process
     *
     *  @param  mapper  The maximum number of records per mapper
     *  @return Was the setting stored (only possible if not started already)
     */
    bool maxrecords(int64_t mapper)
    {
        // not possible if job has already started
        if (_started) return false;

        // set in the json
        _json.maxrecords(mapper);

        // done
        return true;
    }

    /**
     *  Setter for whether or not to run locally.
     *  @param  value
     *  @return bool
     */
    bool local(bool value)
    {
        // not possible if job has already started
        if (_started) return false;

        // set in the json
        _json.local(value);

        // done
        return true;
    }

    /**
     *  Flush the output file.
     *  @return bool
     */
    bool flush()
    {
        // not possible if already started
        if (_started) return false;

        // simply get a new output
        output(true);

        // done
        return true;
    }

    /**
     *  Add data to the process
     *  @param  data
     *  @return bool
     */
    bool add(const std::string &data)
    {
        // impossible if already started and in the newer versions, this should be replaced with an empty key for example
        if (_started) return false;

        // the output file to which we're going to write
        auto *out = output();

        // do we have such a file?
        if (out)
        {
            // write a record to the file (record ID 0 means that no server
            // or file is available)
            Yothalot::Record record(0);

            // add the data
            record.add(data);

            // put this in the output file
            out->add(record);
        }
        else
        {
            // add data to the json because no directory is available
            _json.add(data);
        }

        // done
        return true;
    }

    /**
     *  Add data to the process in the same files.
     *  @param  key
     *  @param  value
     */
    bool add(const Yothalot::Key &key, const Yothalot::Value &value, const char *server)
    {
        // impossible if already started (is it?)
        if (_started || !isMapReduce()) return false;

        // get the output file we're going to write
        auto out = output();

        // if we have an output, write the record in the output file
        if (out) out->add(Yothalot::Record(Yothalot::KeyValue(key, value)));

        // otherwise we add it to the json directly (will be in separate files)
        else _json.kv(key, value, server);

        // we've successfully added it
        return true;
    }

    /**
     *  This will add all data to different files
     *  @param  key
     *  @param  value
     */
    bool map(const Yothalot::Key &key, const Yothalot::Value &value, const char *server)
    {
        // impossible if already started (is it?) and impossible in older versions (we cannot mishmash the files because
        // they use different protocols. stick to one)
        if (_started || !isMapReduce()) return false;

        // have we been unserialized? in that case adding data to the json
        // is likely not going to work, since the job will be started from
        // somewhere else, that has a different copy of the json by itself
        if (!_core)
        {
            // add the data to an output file
            output()->add(Yothalot::Record{ Yothalot::KeyValue{ key, value }});
        }
        else
        {
            // only add it to the json, faster for distinct files
            _json.kv(key, value, server);
        }

        // we've successfully added it
        return true;
    }

    /**
     *  Add a file to the process
     *  @param  filename
     *  @param  start
     *  @param  size
     *  @param  server
     *  @return bool
     */
    bool file(const char *filename, size_t start, size_t size, bool remove, const char *server)
    {
        // cannot add the file if already started
        if (_started) return false;

        // in this case, we have to use the json to transfer the data
        _json.file(filename, start, size, remove, server);

        // we succeeded
        return true;
    }

    /**
     *  Add a directory to the process
     *  @param  dirname
     *  @param  remove
     *  @return bool
     */
    bool directory(const char *dirname, bool remove, const char *server)
    {
        // cannot add the file if already started
        if (_started) return false;

        // in this case, we have to use the json to transfer the data
        _json.directory(dirname, remove, server);

        // we succeeded
        return true;
    }


    /**
     *  Start the job - was the job started?
     *  This method also returns true if the job was already started in the past.
     *  @return bool
     */
    bool start()
    {
        // if we already started or are done we bail out
        if (_started || _done) return true;

        // creating the temp queue might end up in an exception if no RabbitMQ connection is available
        try
        {
            // we need a temporary queue, because we might need to wait for the answer
            _tempqueue.reset(new TempQueue(this, _core));

            // if we have an output object, we remove it to enforce that all data is flushed
            _output.reset();

            // store the name of the temp queue in the JSON
            _json.tempqueue(_tempqueue->name());

            // now we can publish the job JSON data to the RabbitMQ server
            if (_json.publish(_core.get())) return _started = true;
            
            // destruct the tempqueue
            _tempqueue = nullptr;

            // the weird situation is that we can not connect to RabbitMQ...
            // (really weird because we did manage to create the temp queue...)
            return false;
        }
        catch (...)
        {
            // failure (no need to report to PHP space, as that is already done
            // inside the Core class
            return false;
        }
    }
    
    /**
     *  Is the job ready?
     *  @return bool
     */
    bool ready() const
    {
        return _done;
    }

    /**
     *  Wait for the job to be ready
     *  @return bool
     */
    bool wait()
    {
        // if the job is already done
        if (_done) return !_error;

        // make sure the job is started
        if (!start()) return false;

        // if there is no temp queue, the job was detached, and we cannot wait
        if (_tempqueue == nullptr) return false;

        // wait for the result to appear in the temporary result queue
        _tempqueue->wait();

        // by now we know that we're done
        return !_error;
    }

    /**
     *  Retrieve the result that was generated
     *  @return JSON::Object
     */
    const JSON::Object &result() const
    {
        // expose member
        return _result;
    }

    /**
     *  Detach the job - dont wait for it
     *  @return bool
     */
    bool detach()
    {
        // if we already started or are done we bail out
        if (_done) return false;

        // do we have a temp queue? if so we should get rid of it
        if (_tempqueue) _tempqueue = nullptr;

        // if the job was already started, nothing is left to do
        if (_started) return true;

        // if the job was not yet started, we should do that now
        if (!_json.publish(_core.get())) return false;

        // mark job as started
        return _started = true;
    }

    /**
     *  Expose the JSON job data
     *  @return JSON::Object
     */
    const JSON::Object &json() const
    {
        return _json;
    }

    /**
     *  The underlying connection
     *  @return Core
     */
    const std::shared_ptr<Core> &core() const
    {
        return _core;
    }
};
