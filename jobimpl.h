/**
 *  JobImpl.h
 *
 *  Because the PHP constructor runs _after_ the C++ constructor,
 *  we store all data that should not directly be constructed in
 *  a nested helper class, which is constructed in the __construct()
 *  or unserialize() methods
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
#include "data.h"
#include "tempqueue.h"

/**
 *  Class definition
 */
class JobImpl
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
     *  @var std::shared_ptr<JSON::Object>
     */
    std::shared_ptr<JSON::Object> _result;

    /**
     *  Get access to the output file
     *  @return Yothalot::Output
     */
    Yothalot::Output *output()
    {
        // do we already have such a file?
        if (_output) return _output.get();

        // file is not yet opened, but we need a directory on glusterfs for that
        if (!_directory) return nullptr;

        // construct a new output file
        _output.reset(new Yothalot::Output(std::string(_directory->full()) + "/" + Php::call("uniqid").stringValue()));

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

            // the directory exists, set this in the json
            _json.directory(_directory->relative());
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
        // does the input json contain a specific directory?
        if (!_json.directory()) return;

        // input json contains a directory, construct this (watch out!
        // this might throw an exception if the dir is invalid/can not
        // be read
        _directory.reset(new Directory(_json.directory()));
    }

    /**
     *  Destructor
     */
    virtual ~JobImpl() {}

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
    bool maxfiles(int value)
    {
        // not possible if job has already started
        if (_started) return false;

        // set in the json
        _json.maxfiles(value);

        // done
        return true;
    }

    /**
     *  setter for the max number of bytes
     *  @param value
     *  @return bool
     */
    bool maxbytes(uint64_t value)
    {
        // not possible if job has already started
        if (_started) return false;

        // set in the json
        _json.maxbytes(value);

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
        // impossible if already started
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
     *  Add a file to the process
     *  @param  data
     *  @return bool
     */
    bool file(const std::string &data, const std::string &filename)
    {
        // we cannot add a file if job started
        if (_started) return false;

        // the output file to which we are going to write
        auto *out = output();

        // do we have this file?
        if (out)
        {
            // write a record to the file (record ID 1 means that a filename
            // is available in the second record field)
            Yothalot::Record record(1);

            // add the data
            record.add(data);

            // add the filename;
            record.add(filename);

            // put this in the output file
            out->add(record);
        }
        else
        {
            // add data to the json since no filename is available
            _json.file(data, filename);
        }

        // done
        return true;
    }

    /**
     *  Add a server to the process
     *  @param  data
     *  @return bool
     */
    bool server(const std::string &data, const std::string &servername)
    {
        // we cannot add a server if job started
        if (_started) return false;

        // the output file to which we are going to write
        auto *out = output();

        // do we have this file?
        if (out)
        {
            // write a record to the file (record ID 2 means that a sever name
            // is available in the second record field)
            Yothalot::Record record(2);

            // add the data
            record.add(data);

            // add the servername;
            record.add(servername);

            // put this in the output file
            out->add(record);
        }
        else
        {
            // add data to the json since no filename is available
            _json.server(data, servername);
        }

        // done
        return true;
    }

    /**
     *  Start the job
     *  @return bool
     */
    bool start()
    {
        // if we already started or are done we bail out
        if (_started || _done) return false;

        // creating the temp queue might end up in an exception if no RabbitMQ connection is available
        try
        {
            // we need a temporary queue, because we might need to wait for the answer
            _tempqueue.reset(new TempQueue(_core));

            // if we have an output object, we remove it to enforce that all data is flushed
            _output = nullptr;

            // store the name of the temp queue in the JSON
            _json.tempqueue(_tempqueue->name());

            // now we can publish the job JSON data to the RabbitMQ server
            if (_core->publish(_json)) return _started = true;

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
     *  Wait for the job to be ready
     *  @return bool
     */
    bool wait()
    {
        // if already ready
        if (_done) return true;

        // we can only wait for a job if it was started
        if (!_started && !start()) return false;

        // we should have a temporary queue, otherwise there is no way how we can wait for an answer
        if (!_tempqueue) return false;

        // consume the message from the temporary queue
        _result = std::make_shared<JSON::Object>(_tempqueue->consume());

        // did an error occur?
        if (!_result->contains("stderr")) return true;

        // report the error to PHP space as a warning
        Php::warning << _result->c_str("stderr") << std::flush;

        // and return our result (which is not good)
        return false;
    }

    /**
     *  Retrieve the result that was generated
     *  @return std::shared_ptr
     */
    const std::shared_ptr<JSON::Object> &result() const
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
        if (!_core->publish(_json)) return false;

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
    std::shared_ptr<Core> core() const
    {
        return _core;
    }
};