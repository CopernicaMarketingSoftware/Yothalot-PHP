/**
 *  Data.h
 *
 *  Class that represents the JSON object that holds all JOB data
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
#include "json/array.h"
#include <phpcpp.h>

/**
 *  Class definition
 */
class Data : public JSON::Object, private TupleHelper
{
private:
    /**
     *  If the job is started on a node that is not mounter to glusterFS,
     *  the data is going to be stored in the JSON as well
     *  @var JSON::Array
     */
    JSON::Array _input;

    /**
     *  What sort of algorithm are we going to run?
     */
    enum {
        algorithm_mapreduce,
        algorithm_race,
        algorithm_job
    } _algorithm = algorithm_job;

    /**
     *  Utility class for an executable (the mapper, reducer or finalizer
     */
    class Executable : public JSON::Object
    {
    public:
        /**
         *  Constructor
         *  @param  name        Name of the executable (mapper, reducer or finalizer)
         *  @param  stdin       Data to be set as "stdin" property
         */
        Executable(const char *name, const std::string &stdin)
        {
            // construct the full name
            std::string fullname("exit(YothalotInit('");
            fullname.append(name).append("'));");

            // set all the properties
            set("executable", "php");
            set("arguments", JSON::Array({"-r", fullname.data()}));
            set("stdin", stdin);
            set("limit", JSON::Object());
        }

        /**
         *  Destructor
         */
        virtual ~Executable() {}
    };

    /**
     *  Class definition to create a simple stdin for races
     */
    class InputData : public std::string
    {
    public:
        /**
         *  Constructor
         *  @param  algo        User supplied algorithm object
         */
        InputData(const Php::Value &algo)
        {
            // serialize the user-supplied object
            auto serialized = Php::call("serialize", algo);

            // find out what the include files are
            auto includes = algo.call("includes");

            // create a simple php array with the includes and the algorithm object
            Php::Value array(Php::Type::Array);
            array[0] = includes;
            array[1] = serialized;

            // serialize the array, and base64 encode it to ensure that we have no NULL values in the string
            auto result = Php::call("base64_encode", Php::call("serialize", array));

            // assign the result to the std::string
            assign(result.stringValue());

            // append some newlines as we should be followed by data
            append("\n\n");
        }
    };

public:
    /**
     *  Constructor
     *  @param  algo       User-supplied algorithm object
     */
    Data(const Php::Value &algo)
    {
        // construct the input data
        InputData input(algo);

        // in case we're a map reduce algorithm we set a modulo, mapper, reducer and writer
        if (algo.instanceOf("Yothalot\\MapReduce"))
        {
            // set default limits
            set("processes", 20);
            set("input", _input);
            set("modulo", 1);
            set("mapper", Executable("mapper", input));
            set("reducer", Executable("reducer", input));
            set("finalizer", Executable("finalizer", input));
            set("version", 1);

            // remember algorithm type
            _algorithm = algorithm_mapreduce;
        }

        // the kvmapreduce is new, and version 2
        else if (algo.instanceOf("Yothalot\\MapReduce2"))
        {
            // set default limits
            set("processes", 20);
            set("input", _input);
            set("modulo", 1);
            set("version", 2);
            set("mapper", Executable("kvmapper", input));
            set("reducer", Executable("reducer", input));
            set("finalizer", Executable("finalizer", input));

            // remember algorithm type
            _algorithm = algorithm_mapreduce;
        }

        // in case we are a race we just set an executable manually etc.
        else if (algo.instanceOf("Yothalot\\Race"))
        {
            // set the json properties
            set("executable", "php");
            set("arguments", JSON::Array({"-r", "exit(YothalotInit('run'));"}));
            set("stdin", input);
            set("input", _input);

            // remember algorithm type
            _algorithm = algorithm_race;
        }
    }

    /**
     *  Constructor for unserialized input data
     *  @param  object          The unserialized JSON object
     */
    Data(const JSON::Object &object) :
        JSON::Object(object),
        _input(object.array("input")) {}

    /**
     *  Destructor
     */
    virtual ~Data() {}

    /**
     *  Simple checkers for race and mapreduce
     *  @return bool
     */
    bool isRace() const { return _algorithm == algorithm_race; }
    bool isMapReduce() const { return _algorithm == algorithm_mapreduce; }

    /**
     *  Publish the data to a connection
     *  @param  connection
     */
    bool publish(Core *connection) const
    {
        switch (_algorithm) {
        case algorithm_mapreduce:   return connection->mapreduce(*this);
        case algorithm_race:        return connection->race(*this);
        case algorithm_job:         return connection->job(*this);
        default:                    return false;
        }
    }

    /**
     *  The directory that is set in the data
     *  @return std::string
     */
    const char *directory() const
    {
        return isString("input") ? c_str("input") : nullptr;
    }

    /**
     *  Set the directory (this must be a _relative_ path!)
     *  @param  path
     */
    void directory(const char *path)
    {
        // update json
        set("input", path);
    }

    /**
     *  Update max number of processes in the JSON
     *  @param  value
     */
    void maxprocesses(int value)
    {
        // update json
        set("processes", value);
    }

    /**
     *  Update max number of mappers in the JSON
     *  @param value
     */
    void maxmappers(int value)
    {
        // update JSON
        object("mapper").object("limit").set("processes", value);
    }

    /**
     *  Update max number of reducers in the JSON
     *  @param value
     */
    void maxreducers(int value)
    {
        // update JSON
        object("reducer").object("limit").set("processes", value);
    }

    /**
     *  Update max number of finalizers
     *  @param  value
     */
    void maxfinalizers(int value)
    {
        // update JSON
        object("finalizer").object("limit").set("processes", value);
    }

    /**
     *  Update the modulo in the JSON
     *  @param value
     */
    void modulo(int value)
    {
        // update JSON
        set("modulo", value);
    }

    /**
     *  Set the max number of files
     *  @param  value
     */
    void maxfiles(int64_t mapper, int64_t reducer, int64_t finalizer)
    {
        // set the limit files property on the mapper, reducer and finalizer
        if (mapper) object("mapper").object("limit").set("files", mapper);
        if (reducer) object("reducer").object("limit").set("files", reducer);
        if (finalizer) object("finalizer").object("limit").set("files", finalizer);
    }

    /**
     *  Set the max number of bytes
     *  @param value
     */
    void maxbytes(int64_t mapper, int64_t reducer, int64_t finalizer)
    {
        // set the limit files property on the mapper, reducer, and finalizer
        if (mapper) object("mapper").object("limit").set("bytes", mapper);
        if (reducer) object("reducer").object("limit").set("bytes", reducer);
        if (finalizer) object("finalizer").object("limit").set("bytes", finalizer);
    }

    /**
     *  Set the local property
     *  @param  value
     */
    void local(bool value)
    {
        // set the local property
        set("local", value);
    }

    /**
     *  Add input data to the json
     *  @param  data
     */
    void add(const std::string &data)
    {
        // initialize a simple json object
        JSON::Object object;

        // set the data property
        object.set("data", data);

        // and move the data into the input array
        _input.append(std::move(object));

        // set the changed input
        set("input", _input);
    }

    /**
     *  Add file data to the json
     *  @param  filename
     *  @param  start
     *  @param  size
     */
    void file(const char *filename, size_t start, size_t size, bool remove, const char *server)
    {
        // initialize a simple json object
        JSON::Object object;

        // set the filename
        object.set("filename", filename);

        // set the file start
        object.set("start", (int64_t)start);

        // set the size
        object.set("size", (int64_t)size);

        // set whether or not to remove
        object.set("remove", remove);

        // set the server if present
        if (server && *server != 0) object.set("server", server);

        // move the data into the input array
        _input.append(std::move(object));

        // set the changed input
        set("input", _input);
    }

    /**
     *  Add key/value data to the json
     *  @param  key         The key to add
     *  @param  value       The value to add
     *  @param  server      Default empty.
     */
    void kv(const Yothalot::Key &key, const Yothalot::Value &value, const char *server)
    {
        // initialize a simple json object
        JSON::Object object;

        // set the key in the json
        object.set("key", toJson(key));

        // set the value in the json
        object.set("value", toJson(value));

        // set the server if present
        if (server && *server != 0) object.set("server", server);

        // move the data into the input array
        _input.append(std::move(object));

        // set the changed input
        set("input", _input);
    }

    /**
     *  Add a directory to the json nonintrusively
     *  @param  directory       The directory to add
     *  @param  server          The server to add the directory to
     */
    void directory(const char *dirname, bool remove, const char *server)
    {
        // initialize a simple json object
        JSON::Object object;

        // set the dirname
        object.set("dirname", dirname);

        // set whether or not to remove
        object.set("remove", remove);

        // set the server if present
        if (server && *server != 0) object.set("server", server);

        // move the data into the input array
        _input.append(std::move(object));

        // set the changed input
        set("input", _input);
    }

    /**
     *  Get the version
     *  @return int
     */
    int version() const
    {
        // get the info from the json
        return integer("version");
    }

    /**
     *  Set the name of the temporary queue
     *  @param  name
     */
    void tempqueue(const std::string &name)
    {
        // two properties are necessary
        set("exchange", "");
        set("routingkey", name);
    }
};

