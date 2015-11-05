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
class Data : public JSON::Object
{
private:
    /**
     *  If the job is started on a node that is not mounter to glusterFS,
     *  the data is going to be stored in the JSON as well
     *  @var JSON::Array
     */
    JSON::Array _input;

    /**
     *  Helper class that creates an array with the to-be-included files
     */
    class Includes : public JSON::Array
    {
    public:
        /**
         *  Constructor
         *  @param  obj       User-supplied map/reduce or race object
         */
        Includes(const Php::Value &obj)
        {
            // call the includes method on our map reduce object
            Php::Value includes = obj.call("includes");

            // check for a single string and for an array
            if (includes.isString()) append(includes.rawValue(), includes.size());
            else if (includes.isArray())
            {
                // loop over all the files in the includes array
                for (auto iter : includes)
                {
                    // if it's a string we're appending it
                    if (iter.second.isString()) append(iter.second.stringValue());
                }
            }
        }

        /**
         *  Destructor
         */
        virtual ~Includes() {}
    };

    /**
     *  Utility class for an executable (the mapper, reducer or finalizer
     */
    class Executable : public JSON::Object
    {
    public:
        /**
         *  Constructor
         *  @param  name        Name of the executable (mapper, reducer or finalizer)
         *  @param  includes    The to-be-included files
         *  @param  serialized  Serialized data
         */
        Executable(const char *name, const Includes &includes, const std::string &serialized)
        {
            // construct the full name
            std::string fullname("exit(YothalotInit('");
            fullname.append(name).append("'));");

            // set all the properties
            set("executable", "php");
            set("arguments", JSON::Array({"-r", fullname.data()}));
            set("object", serialized);
            set("includes", includes);
            set("limit", JSON::Object());
        }

        /**
         *  Destructor
         */
        virtual ~Executable() {}
    };

public:
    /**
     *  Constructor
     *  @param  algo       User-supplied algorithm object
     */
    Data(const Php::Value &algo)
    {
        // in case we're a map reduce algorithm we set a modulo, mapper, reducer and writer
        if (algo.instanceOf("Yothalot\\MapReduce"))
        {
            // serialize the object
            auto serialized = Php::call("base64_encode", Php::call("serialize", algo)).stringValue();

            // construct the includes
            Includes includes(algo);

            // set default limits
            set("processes", 20);
            set("input", _input);

            set("modulo", 1);
            set("mapper", Executable("mapper", includes, serialized));
            set("reducer", Executable("reducer", includes, serialized));
            set("finalizer", Executable("finalizer", includes, serialized));
        }
        // in case we are a racer we just set an executable manually etc.
        else if (algo.instanceOf("Yothalot\\Racer"))
        {
            // create an object with the two properties
            Php::Value input(Php::Type::Array);
            input[0] = algo.call("includes");
            input[1] = Php::call("serialize", algo); // this one is serialized twice as we're unable to
                                                     // unserialize it the first time as the includes aren't included yet at that point

            // serialize the input
            auto serialized = Php::call("base64_encode", Php::call("serialize", input));

            // set the json properties
            set("executable", "php");
            set("arguments", JSON::Array({"-r", "exit(YothalotInit('run'));"}));
            set("stdin", serialized.stringValue() + "\n\n");
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
    void maxfiles(int64_t value)
    {
        // set the limit files property on the mapper, reducer and finalizer
        object("mapper").object("limit").set("files", value);
        object("reducer").object("limit").set("files", value);
        object("finalizer").object("limit").set("files", value);
    }

    /**
     *  set the max number of bytes
     *  @param value
     */
    void maxbytes(int64_t value)
    {
        // set the limit files property on the mapper, reducer, and finalizer
        object("mapper").object("limit").set("bytes", value);
        object("reducer").object("limit").set("bytes", value);
        object("finalizer").object("limit").set("bytes", value);
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
     *  @param data
     *  @param filename
     */
    void file(const std::string &data, const std::string &filename)
    {
        // initialize a simple json object
        JSON::Object object;

        // set the data property
        object.set("data", data);

        // set the filename
        object.set("filename", filename);

        // and move the data into the input array
        _input.append(std::move(object));

        // set the changed input
        set("input", _input);
    }

    /**
     *  Add server data to the json
     *  @param data
     *  @param servername
     */
    void server(const std::string &data, const std::string &servername)
    {
        // initialize a simple json object
        JSON::Object object;

        // set the data property
        object.set("data", data);

        // set the server name
        object.set("server", servername);

        // and move the data into the input array
        _input.append(std::move(object));

        // set the changed input
        set("input", _input);
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

