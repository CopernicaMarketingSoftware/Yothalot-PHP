/**
 *  Init.cpp
 *
 *  The init function that should be called directly on cli.
 *
 *  @author    Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include "init.h"
#include "job.h"
#include "wrapper.h"
#include "base.h"
#include <yothalot.h>

/**
 *  Run the mapper
 *  @param  object          User supplied MapReduce object
 *  @param  input           All the input
 *  @return int
 */
static int map(Php::Value &&object, Yothalot::Stdin &input)
{
    // prevent exceptions
    try
    {
        // wrap the php object
        Wrapper mapreduce(std::move(object));

        // prevent PHP output during the map/reduce algorithm
        Php::call("ob_start");

        // input data must be set
        if (!input.data()) throw std::runtime_error("No input data is available");

        // directory must be set
        if (!input.directory()) throw std::runtime_error("No output directory is available");

        // create the task
        Yothalot::MapTask task(base(), &mapreduce, input.modulo(), input.directory());

        // add the data to process
        task.process(input.data(), strlen(input.data()));

        // capture the output
        std::string output = Php::call("ob_get_clean");

        // did we have output?
        if (output.size() > 0) throw std::runtime_error(output);

        // show output of mapper process
        std::cout << task.output();

        // done
        return 0;
    }
    catch (const std::runtime_error &error)
    {
        // report error
        Php::error << "Mapper error: " << error.what() << std::flush;

        // failure
        return -1;
    }
}

/**
 *  Run the reducer
 *  @param  object          User supplied MapReduce object
 *  @param  input           All the input
 *  @return int
 */
static int reduce(Php::Value &&object, Yothalot::Stdin &input)
{
    // prevent exceptions
    try
    {
        // wrap php object
        Wrapper mapreduce(std::move(object));

        // prevent PHP output during the map/reduce algorithm
        Php::call("ob_start");

        // directory must be set
        if (!input.directory()) throw std::runtime_error("No output directory is available");

        // create the task
        Yothalot::ReduceTask task(base(), &mapreduce, input.index(), input.directory());

        // distribute the input files
        input.distribute(&task);

        // capture the output
        std::string output = Php::call("ob_get_clean");

        // did we have output?
        if (output.size() > 0) throw std::runtime_error(output);

        // show output of mapper process
        std::cout << task.output();

        // done
        return 0;
    }
    catch (const std::runtime_error &error)
    {
        // report error
        Php::error << "Reducer error: " << error.what() << std::flush;

        // failure
        return -1;
    }
}

/**
 *  Run the writer/finalizer
 *  @param  object          User supplied MapReduce object
 *  @param  input           All the input
 *  @return int
 */
static int write(Php::Value &&object, Yothalot::Stdin &input)
{
    // prevent exceptions
    try
    {
        // wrap php object
        Wrapper mapreduce(std::move(object));

        // prevent PHP output during the map/reduce algorithm
        Php::call("ob_start");

        // create the task
        Yothalot::WriteTask task(base(), &mapreduce);

        // distribute the input files
        input.distribute(&task);

        // capture the output
        std::string output = Php::call("ob_get_clean");

        // did we have output?
        if (output.size() > 0) throw std::runtime_error(output);

        // show output of mapper process
        std::cout << task.output();

        // done
        return 0;
    }
    catch (const std::runtime_error &error)
    {
        // report error
        Php::error << "Writer error: " << error.what() << std::flush;

        // failure
        return -1;
    }
}

/**
 *  Run the racer
 *  @param  object      User supplied Race object
 *  @param  input       All the input
 *  @return int
 */
static int run(Php::Value &&object, Yothalot::Stdin &input)
{
    // prevent exceptions
    try
    {
        // prevent PHP output during race algorithm
        Php::call("ob_start");

        // wrap all input data in a php string variable (should we serialize here?)
        auto data = Php::call("unserialize", Php::call("base64_decode", Php::Value(input.data(), strlen(input.data()))));

        // call the process method
        auto result = object.call("process", data);

        // if there's no output, the job generated no output
        if (result.isNull()) return 0;

        // capture the output
        std::string output = Php::call("ob_get_clean");

        // did we have output?
        if (output.size() > 0) throw std::runtime_error(output);

        // serialize the output, so that it can be unserialized at the caller side
        std::cout << Php::call("base64_encode", Php::call("serialize", result));

        // done
        return 0;
    }
    catch (const std::runtime_error &error)
    {
        // report the error
        Php::error << "Unexpected output: " << error.what() << std::flush;

        // failure
        return -1;
    }
}

/**
 *  Our global init method, mostly used to call directly from cli using something
 *  like `php -r "YothalotInit('mapper');"`
 *  @param  params
 *  @return Php::Value  Return values are like normal programs really, 0 if success
 *                      something else otherwise.
 */
Php::Value yothalotInit(Php::Parameters &params)
{
    // turn on all errors
    Php::error_reporting(Php::Error::All);
    Php::call("ini_set", "error_log", nullptr); // disable the error_log
    Php::call("ini_set", "display_errors", "stderr");

    // read all data from input
    Yothalot::Stdin input;

    // parse the json
    JSON::Object json(input.json());

    // what files should we be including?
    JSON::Array includes = json.array("includes");

    // loop over all our includes and call require_once with all of them
    for (int i = 0; i < includes.size(); ++i)
    {
        // required file must be a string
        if (!includes.isString(i)) Php::error << "Include file is not a string" << std::flush;

        // to prevent that the "require_once" statement echo's to stdout, we start output capturing
        Php::call("ob_start");

        // include the PHP file (this could cause a PHP fatal error)
        if (!Php::include_once(includes.c_str(i))) Php::error << "Failed to include " << includes.c_str(i) << std::flush;

        // capture the output
        auto output = Php::call("ob_get_clean");

        // we expect the output to be empty
        if (output.size() == 0) continue;

        // the PHP file is not supposed to create output, show this as error
        Php::error << "Output was generated by included file " << includes.c_str(i) << " (" << output << ")" << std::flush;
    }

    // the "unserialize" algorithm could be written by the user, in which he accidentally left
    // some output-generating code (which we dont want), capture this output
    Php::call("ob_start");

    // try to unserialize the original object
    Php::Value unserialized = Php::call("unserialize", Php::call("base64_decode", json.c_str("object")));

    // capture the output
    auto output = Php::call("ob_get_clean");

    // we expect the output to be empty
    if (output.size() > 0) Php::error << "Output was generated when unserializing object (" << output << ")" << std::flush;

    // check the type of task to run
    if      (strcasecmp(params[0].rawValue(), "mapper")    == 0) return map(std::move(unserialized), input);
    else if (strcasecmp(params[0].rawValue(), "reducer")   == 0) return reduce(std::move(unserialized), input);
    else if (strcasecmp(params[0].rawValue(), "finalizer") == 0) return write(std::move(unserialized), input);
    else if (strcasecmp(params[0].rawValue(), "run")       == 0) return run(std::move(unserialized), input);

    // program was started in unknown mode
    Php::error << "Unrecognized input mode " << params[0] << std::flush;

    // done with failure
    return -1;
}
