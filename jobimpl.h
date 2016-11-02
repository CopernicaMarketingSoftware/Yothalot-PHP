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
#include "target.h"
#include "notnull.h"
#include "workingdir.h"

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
     *  @var std::shared_ptr<Rabbit>
     */
    std::shared_ptr<Rabbit> _rabbit;
    
    /**
     *  Shared pointer to the core cache settings
     *  @var std::shared_ptr<Cache>
     */
    std::shared_ptr<Cache> _cache;

    /**
     *  State of the job
     */
    enum {
        state_initialize,       // job is being initialized, it is still possible to change settings and add input
        state_frozen,           // job has been frozen because it was (un)serialized, the input data in the json can no longer be altered
        state_running,          // job is busy running
        state_finished,         // job finished running
    } _state;

    /**
     *  The temporary queue the result will be published to
     *  @var TempQueue
     */
    std::unique_ptr<TempQueue> _tempqueue;

    /**
     *  The temporary directory holding all input data
     *  @var Directory
     */
    Directory _directory;

    /**
     *  Yothalot target object
     *  @var Target
     */
    Target _target;

    /**
     *  The file to which records are written.
     *  @var Yothalot::Output
     */
    std::unique_ptr<Yothalot::Output> _datafile;

    /**
     *  Result data sent back
     *  @var JSON::Object
     */
    JSON::Object _result;

    
    /**
     *  Was the job an error
     *  @return bool
     */
    bool isError() const
    {
        // job must be finished
        if (_state != state_finished) return false;
        
        // if we did not get an object back, there must be something wrong
        if (_result.size() == 0) return true;

        // in case case of a task we may have the stderr in the main json right away
        if (isTask() && _result.contains("stderr")) return true;

        // if we don't have an error we return true
        if (_result.contains("error")) return true;
        
        // this was not an error
        return false;
    }
    
    /**
     *  Is the object still tunable?
     *  @return bool
     */
    bool isTunable() const
    {
        // this is possible
        return _state == state_initialize || _state == state_frozen;
    }

    /**
     *  Function to run the finalization process on the client
     *  @param  directory
     *  @return bool
     */
    bool finalize(const Directory &directory)
    {
        // prevent exceptions (the working dir object could for example throw)
        try
        {
            // hey. the finalizer did not yet run on the yothalot cluster, that means that we
            // have to do the finalizing in this process
            Wrapper mapreduce(_json.finalizer());
            
            // change working dir (the destructor will change back to current dir)
            WorkingDir workingdir(directory.full());
        
            // create the write task
            Yothalot::WriteTask task(base(), &mapreduce, _cache->connection(), true);

            // get the input
            auto input = _result.array("finalize");
            
            // traverse over the input for the finalize processes
            for (int i = 0; i < input.size(); ++i)
            {
                // input for the finalizer
                const char *data = input.c_str(i);
                
                // skip if it was not a string
                if (data == nullptr) continue;
                
                // pass to the task
                task.process(data, strlen(data));
            }

            // done
            return true;
        }
        catch (const std::runtime_error &error)
        {
            // something went terribly wrong
            return false;
        }
    }
    
    /**
     *  Called when result comes in
     *  @param  queue
     *  @param  buffer
     *  @param  size
     */
    virtual void onReceived(TempQueue *queue, const char *buffer, size_t size) override
    {
        // change state
        _state = state_finished;
        
        // assign to the result variable
        _result = JSON::Object(buffer, size);

        // nothing left to do on error
        if (isError()) return;
        
        // nothing left to be done when this is not a map-reduce job
        if (!isMapReduce() || _result.object("finalizers").integer("processes") > 0) return;

        // name of the directory that contains the result files (this directory is normally not
        // exposed, but if we have to run the finalizer ourselves, it sometimes is)
        const char *directory = _result.c_str("directory");
            
        // leap out if there is no directory with files
        if (directory == nullptr || directory[0] == 0) return;

        // construct the directory object
        Directory dir(directory);
        
        // do we have to finalize the data ourselves?
        if (_result.isArray("finalize")) finalize(dir);
        
        // remove the directory
        dir.remove();
    }
    
    /**
     *  Called in case of an error
     *  @param  queue
     *  @param  message
     */
    virtual void onError(TempQueue *queue, const char *message) override
    {
        // remember that we're in an error state
        _state = state_finished;
    }
    
    /**
     *  Install a new output file
     *  @param  file
     *  @return Yothalot::Output
     */
    Yothalot::Output *install(Yothalot::Output *file)
    {
        // install in the unique-ptr
        _datafile.reset(file);
        
        // done
        return file;
    }

    /**
     *  Get access to the input file
     *  @return Yothalot::Output
     */
    Yothalot::Output *datafile()
    {
        // do we already have such a file?
        if (_datafile != nullptr) return _datafile.get();
        
        // exceptions could occur if file could not be created
        try
        {
            // if we're still initializing, and this is the only object with access to the
            // json, we can still construct datafiles that are either stored in nosql or on disk
            if (_state == state_initialize) return install(new Yothalot::Output(&_target));

            // the only situation that we can deal is when the object is frozen, the other
            // cases (process is already running or completed) do not allow adding extra data
            if (_state != state_frozen) return nullptr;

            // the job object has already been serialized, which means that multiple
            // instances have access to the data, and that we can no longer update
            // the json (because we dont know which script is leading), we must use
            // a file-based job object, make sure that the directory exists
            _directory.create();
                
            // create new file-based output object
            return install(new Yothalot::Output(std::string(_directory.full()) + "/" + (std::string)Yothalot::UniqueName(), true));
        }
        catch (...)
        {
            // no datafile available
            return nullptr;
        }
    }

    /**
     *  Synchronize the datafile, so that the json is up-to-date
     *  @param  keep        keep a file reference in memory, more data could follow
     *  @return bool
     */
    bool sync(bool keep)
    {
        // if there is no data file, there is nothing to flush
        if (_datafile == nullptr) return false;
        
        // we have a data file, flush it
        _datafile->flush();
        
        // the datafile, is it stored in nosql or in a regular file?
        if (strncasecmp(_datafile->name().data(), "cache://", 8) == 0)
        {
            // the datafile is saved as an object in nosql. However, we are 
            // only going to pass a directory to the yothalot master process,
            // so we have to include this nosql address explicitly in the 
            // json input. This can be done as a "cache://" filename
            _json.file(_datafile->name().data(), 0, _datafile->size(), true, nullptr);
            
            // from this moment on, we can no longer use the nosql based data file
            _datafile = nullptr;
        }
        else if (!keep)
        {
            // we do not have to keep a reference to the file
            _datafile = nullptr;
       }

        // done
        return true;
    }


public:
    /**
     *  Constructor for constructing a brand new job
     *  @param  rabbit      The core rabbitmq connection object
     *  @param  cache       The cache settings
     *  @param  algo        User supplied algorithm object
     */
    JobImpl(const std::shared_ptr<Rabbit> &rabbit, const std::shared_ptr<Cache> &cache, const Php::Value &algo) :
        _json(cache.get(), algo),
        _rabbit(rabbit),
        _cache(cache),
        _state(state_initialize),
        _target(_cache, _directory.full())
    {
        // the directory exists, set this in the json, we want the cleanup and no server
        if (_json.isMapReduce()) _json.directory(_directory.relative(), true, nullptr);

        // this is a race job, add the directory directly
        // @todo why are race jobs different?
        else _json.directory(_directory.relative());
    }

    /**
     *  Constructor for an unserialized job
     *  Throws an error if the json did not contain a directory
     *  @param  data        A JSON object holding unserialized data
     *  @throws std::runtime_error
     */
    JobImpl(const JSON::Object &data) :
        _json(data.object("job")),
        _state(state_frozen),
        _directory(NotNull<const char>(_json.directory())),
        _target(_directory.full())
    {
        // we don't create a _rabbit and _cache connections here on purpose, as we just don't need one
        
        // @todo _json.directory() does it return an editable, removable, directory?
        
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
     *  Relative path name of the temporary directory
     *  @return const char
     */
    const char *directory()
    {
        // if someone (in this case: an external php script) is interested in 
        // the directory, it must of course exist
        _directory.create();
        
        // expose the path
        return _directory.relative();
    }

    /**
     *  Setters for max processes
     *  @param  value
     *  @return bool
     */
    bool maxprocesses(int value)
    {
        // only possible if this is the original constructed job object
        if (_state != state_initialize) return false;

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
        // only possible if this is the original constructed job object
        if (_state != state_initialize) return false;

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
        // only possible if this is the original constructed job object
        if (_state != state_initialize) return false;

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
        // only possible if this is the original constructed job object
        if (_state != state_initialize) return false;

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
        // only possible if this is the original constructed job object
        if (_state != state_initialize) return false;

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
        // not possible if job is no longer tunable
        if (!isTunable()) return false;

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
        // not possible if job is no longer tunable
        if (!isTunable()) return false;

        // @todo the byte limit for each mapper *must* be a multiple of the
        // split size of the generated input files, because they are
        // compressed, so we can only read from the start of a split
        //if (mapper    % _splitsize) return false;
        //if (reducer   % _splitsize) return false;
        //if (finalizer % _splitsize) return false;

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
        // not possible if job is no longer tunable
        if (!isTunable()) return false;

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
        // not possible if job is no longer tunable
        if (!isTunable()) return false;

        // set in the json
        _json.local(value);

        // done
        return true;
    }

    /**
     *  Flush the output file, this is also used to indicate the all the
     *  previous emitted key/value pairs or input data should be passed to
     *  a different process (so should be stored in a different file)
     *  @return bool
     */
    bool flush()
    {
        // synchronize the output file
        return sync(false);
    }
    
    /**
     *  Freeze the object, from now on we no longer allow modifications to the
     *  json data, because the job has been serialized, and could be accessed from
     *  multiple processes
     */
    void freeze()
    {
        // synchronize output file (we can keep a reference to the file, because
        // it is ok to add additional data to it)
        sync(true);
        
        // update the state
        _state = state_frozen;
    }

    /**
     *  Add data to the process
     *  @param  data
     *  @return bool
     */
    bool add(const std::string &data)
    {
        // impossible if already started
        if (_state == state_running || _state == state_finished) return false;
        
        // do we have a datafile in which we can store this data?
        auto *file = datafile();
        
        // do we have such a datafile?
        if (file == nullptr)
        {
            // no datafile is available, so we're going to store the data in the json,
            // which is only possible to objects that were original created (and not 
            // unserialized), because unserialized object may share the same json and
            // can not modify it
            if (_state != state_initialize) return false;
            
            // add data to json
            _json.add(data);
        }
        else
        {
            // write a record to the file (record ID 0 means that no server
            // or file is available)
            Yothalot::Record record(0);

            // add the data
            record.add(data);

            // put this in the output file
            file->add(record);
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
        // impossible if already started
        if (_state == state_running || _state == state_finished) return false;

        // adding key/value pairs only makes sense for mapreduce jobs
        if (!isMapReduce()) return false;

        // do we have a datafile in which we can store this data?
        auto *file = datafile();
        
        // do we have such a datafile?
        if (file == nullptr)
        {
            // we can not add data to a shared file, so we must add it to the json,
            // which is not possible if there are multiple jobs around that all refer
            // to the same json
            if (_state != state_initialize) return false;
            
            // add to the json
            _json.kv(key, value, server);
        }
        else
        {
            // add to the file
            file->add(Yothalot::Record(Yothalot::KeyValue(key, value)));
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
        // manipulating json only works for the original constructed object
        if (_state != state_initialize) return false;

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
        // manipulating json only works for the original constructed object
        if (_state != state_initialize) return false;

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
        if (_state == state_running || _state == state_finished) return true;

        // creating the temp queue might end up in an exception if no RabbitMQ connection is available
        try
        {
            // we need a temporary queue, because we might need to wait for the answer
            _tempqueue.reset(new TempQueue(this, _rabbit));

            // store the name of the temp queue in the JSON
            _json.tempqueue(_tempqueue->name());

            // before we start the job, we must ensure that all data is on disk or in nosq
            sync(false);

            // now we must synchronize the json with the datafile that we use (if this is a nosql
            // based datafile, the json has to be updated), and send the job data to RabbitMQ
            if (_json.publish(_rabbit.get())) 
            {
                // the job has been started
                _state = state_running;
                
                // done
                return true;
            }
            else
            {
                // destruct the tempqueue
                _tempqueue = nullptr;

                // the weird situation is that we can not connect to RabbitMQ...
                // (really weird because we did manage to create the temp queue...)
                return false;
            }
        }
        catch (...)
        {
            // failure (no need to report to PHP space, as that is already done
            // inside the Rabbit class
            return false;
        }
    }
    
    /**
     *  Is the job ready?
     *  @return bool
     */
    bool ready() const
    {
        // check state
        return _state == state_finished;
    }

    /**
     *  Wait for the job to be ready
     *  @return bool
     */
    bool wait()
    {
        // if the job is already done
        if (_state == state_finished) return !isError();

        // make sure the job is started
        if (!start()) return false;

        // if there is no temp queue, the job was detached, and we cannot wait
        if (_tempqueue == nullptr) return false;

        // wait for the result to appear in the temporary result queue
        _tempqueue->wait();

        // by now we know that we're done
        return !isError();
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
        if (_state == state_finished) return false;

        // do we have a temp queue? if so we should get rid of it
        if (_tempqueue) _tempqueue = nullptr;

        // if the job was already started, nothing is left to do
        if (_state == state_running) return true;

        // we have to make sure that all data is on disk on in nosql
        sync(false);

        // if the job was not yet started, we should do that now
        if (!_json.publish(_rabbit.get())) return false;

        // mark job as started
        _state = state_running;
        
        // done
        return true;
    }

    /**
     *  Expose the JSON job data
     *  @return JSON::Object
     */
    const JSON::Object &json() const
    {
        // expose json
        return _json;
    }

    /**
     *  The underlying connection
     *  @return Rabbit
     */
    const std::shared_ptr<Rabbit> &rabbit() const
    {
        // expose member
        return _rabbit;
    }
};
