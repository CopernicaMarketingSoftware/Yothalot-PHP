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
#include "stdin.h"
#include <yothalot.h>

/**
 *  Run the mapper
 *  @param  input       All the input
 *  @return int
 */
static int map(const Stdin &input)
{
    // prevent exceptions
    try
    {
        // wrap the php object
        Wrapper mapreduce(input.object());

        // get our argv
        auto argv = Php::GLOBALS["argv"];
        auto argc = Php::GLOBALS["argc"];

        // modulo is the last argument
        auto modulo = argc > 1 ? (int)argv.get(argc - 1) : 1;

        // create the task
        Yothalot::MapTask task(base(), &mapreduce, modulo);

        // add the data to process
        task.process(input.data(), input.size());

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
 *  @param  input           All the input
 *  @return int
 */
static int reduce(const Stdin &input)
{
    // prevent exceptions
    try
    {
        // wrap php object
        Wrapper mapreduce(input.object());

        // create the task
        Yothalot::ReduceTask task(base(), &mapreduce);

        // add the data to process
        task.process(input.data(), input.size());

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
 *  @param  input           All the input
 *  @return int
 */
static int write(const Stdin &input)
{
    // prevent exceptions
    try
    {
        // wrap php object
        Wrapper mapreduce(input.object());

        // create the task
        Yothalot::WriteTask task(base(), &mapreduce);

        // add the data to process
        task.process(input.data(), input.size());

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
 *  Run a regular job (or a job that is part of a race, which is basically identical
 *  to a race job)
 *  @param  input
 *  @return int
 */
static int run(const Stdin &input)
{
    // prevent exceptions
    try
    {
        // call the process method
        auto result = input.object().call("process", Php::call("unserialize", Php::call("base64_decode", Php::Value(input.data(), input.size()))));

        // capture the output
        std::string output = Php::call("ob_get_clean");

        // did we have output?
        if (output.size() > 0) throw std::runtime_error(output);

        // if there's no output, the job generated no output
        if (result.isNull()) return 0;

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

    // prevent PHP output during race algorithm
    Php::call("ob_start");

    // read all input
    Stdin input;

    // result variable
    int result = -1;

    // the run is the very fist simple task
    if (strcasecmp(params[0].rawValue(), "run")            == 0) result = run(input);
    // check the type of task to run that is part of the mapreduce algorithm
    else if (strcasecmp(params[0].rawValue(), "mapper")    == 0 ||
             strcasecmp(params[0].rawValue(), "kvmapper")  == 0) result = map(input);
    else if (strcasecmp(params[0].rawValue(), "reducer")   == 0) result = reduce(input);
    else if (strcasecmp(params[0].rawValue(), "finalizer") == 0) result = write(input);

    // capture the output
    auto output = Php::call("ob_get_clean");

    // we expect the output to be empty
    if (output.size() > 0) Php::error << "Unexpected output (" << output << ")" << std::flush;

    // done with failure
    return result;
}
