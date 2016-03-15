/**
 *  Extension.cpp
 *
 *  Startup file for the PHP extension
 *
 *  @author    Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include <phpcpp.h>

#include "mapreduceresult.h"
#include "raceresult.h"
#include "errorresult.h"
#include "stats.h"
#include "datastats.h"
#include "writer.h"
#include "reducer.h"
#include "values.h"
#include "init.h"
#include "connection.h"
#include "job.h"
#include "path.h"
#include "output.h"
#include "input.h"
#include "record.h"

/**
 *  The VERSION macro is going to be used as string with surrounded quotes
 */
#define STR_VALUE(arg)      #arg
#define VERSION_NAME(name)  STR_VALUE(name)
#define THE_VERSION         VERSION_NAME(VERSION)

/**
 *  tell the compiler that the get_module is a pure C function
 */
extern "C" {

    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  strucure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module()
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("Yothalot", THE_VERSION);

        // create the classes
        Php::Class<Writer>          writer         ("Yothalot\\Writer");
        Php::Class<Reducer>         reducer        ("Yothalot\\Reducer");
        Php::Class<Values>          values         ("Yothalot\\Values");
        Php::Class<Connection>      connection     ("Yothalot\\Connection");
        Php::Class<Job>             job            ("Yothalot\\Job");
        Php::Class<Path>            path           ("Yothalot\\Path");
        Php::Class<Output>          output         ("Yothalot\\Output");
        Php::Class<Input>           input          ("Yothalot\\Input");
        Php::Class<Record>          record         ("Yothalot\\Record");
        Php::Class<MapReduceResult> mapReduceResult("Yothalot\\MapReduceResult");
        Php::Class<RaceResult>      raceResult     ("Yothalot\\RaceResult");
        Php::Class<TaskResult>      taskResult     ("Yothalot\\TaskResult");
        Php::Class<ErrorResult>     errorResult    ("Yothalot\\ErrorResult");
        Php::Class<Stats>           stats          ("Yothalot\\Stats");
        Php::Class<DataStats>       datastats      ("Yothalot\\DataStats");
        Php::Class<Winner>          winner         ("Yothalot\\Winner");

        // register writer functions
        writer.method("emit", &Writer::emit, {
            Php::ByVal("value", Php::Type::Null)
        });

        // register functions on the reducer
        reducer.method("emit", &Reducer::emit, {
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null)
        });

        // register the Connection methods
        connection.method("__construct", &Connection::__construct, {
            Php::ByVal("settings", Php::Type::Array, false)
        }).method("flush", &Connection::flush);

        // register the methods on our php classes
        job.method("__construct", &Job::__construct, {
            Php::ByVal("connection", "Yothalot\\Connection"),
            Php::ByVal("algorithm") // This should be either Yothalot\MapReduce, Yothalot\MapReduce2 or Yothalot\Race
        }).method("add", &Job::add, { // old, v1 on single argument. new, singular keys/values otherwise for more arguments
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null, false),
            Php::ByVal("server", Php::Type::String, false)
        }).method("map", &Job::map, { // this should ensure the multiple case, where mapped values will be combined
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null),
            Php::ByVal("server", Php::Type::String, false)
        }).method("file", &Job::file, { // new, v2 behaviour
            Php::ByVal("filename", Php::Type::String),
            Php::ByVal("start", Php::Type::Numeric, false),
            Php::ByVal("size", Php::Type::Numeric, false),
            Php::ByVal("remove", Php::Type::Bool, false),
            Php::ByVal("server", Php::Type::String, false)
        }).method("directory", &Job::directory, { // new, v2 behaviour
            Php::ByVal("dirname", Php::Type::String, false),
            Php::ByVal("remove", Php::Type::Bool, false),
            Php::ByVal("server", Php::Type::String, false)
        }).method("modulo", &Job::modulo, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxprocesses", &Job::maxprocesses, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxfiles", &Job::maxfiles, { // old behaviour causes all the same, new behaviour has differences
            Php::ByVal("mapper", Php::Type::Numeric),
            Php::ByVal("reducer", Php::Type::Numeric, false),
            Php::ByVal("finalizer", Php::Type::Numeric, false)
        }).method("maxbytes", &Job::maxbytes, { // old behaviour sets all values to the same value, new behaviour set individually
            Php::ByVal("mapper", Php::Type::Numeric),
            Php::ByVal("reducer", Php::Type::Numeric, false),
            Php::ByVal("finalizer", Php::Type::Numeric, false)
        }).method("maxmappers", &Job::maxmappers, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxreducers", &Job::maxreducers, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxfinalizers", &Job::maxfinalizers, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("local", &Job::local, {
            Php::ByVal("value", Php::Type::Bool)
        }).method("flush", &Job::flush, { // new, v2 behaviour
        }).method("start", &Job::start, {
        }).method("detach", &Job::detach, {
        }).method("wait", &Job::wait);

        // register the path methods
        path.method("__construct", &Path::__construct, {
            Php::ByVal("path", Php::Type::String)
        }).method("absolute", &Path::absolute, {
        }).method("relative", &Path::relative);

        // register the output methods
        output.method("__construct", &Output::__construct, {
            Php::ByVal("filename", Php::Type::String),
            Php::ByVal("splitsize", Php::Type::Numeric, false)
        }).method("add", &Output::add, {
            Php::ByVal("identifier", Php::Type::Numeric),
            Php::ByVal("fields", Php::Type::Array)
        }).method("kv", &Output::kv, {
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null)
        }).method("name", &Output::name, {
        }).method("flush", &Output::flush, {
        }).method("size", &Output::size);

        // register input methods
        input.method("__construct", &Input::__construct, {
            Php::ByVal("filename", Php::Type::String),
            Php::ByVal("start", Php::Type::Numeric, false),
            Php::ByVal("bytes", Php::Type::Numeric, false)
        }).method("name", &Input::name, {
        }).method("size", &Input::size)
          .method("valid", &Input::valid)
          .method("next", &Input::next);

        // register record methods
        record.method("identifier", &Record::identifier, {
        }).method("size", &Record::size, {
        }).method("count", &Record::fields, {
        }).method("array", &Record::array, {
        });

        // register map reduce result methods
        mapReduceResult.method("started",    &MapReduceResult::started)
                       .method("runtime",    &MapReduceResult::runtime)
                       .method("mappers",    &MapReduceResult::mappers)
                       .method("reducers",   &MapReduceResult::reducers)
                       .method("finalizers", &MapReduceResult::finalizers);

        // register the race result methods
        raceResult.method("started",         &RaceResult::started)
                  .method("finished",        &RaceResult::finished)
                  .method("runtime",         &RaceResult::runtime)
                  .method("processes",       &RaceResult::processes)
                  .method("result",          &RaceResult::result)
                  .method("winner",          &RaceResult::winner);

        // register the task result methods
        taskResult.method("started",         &TaskResult::started)
                  .method("finished",        &TaskResult::finished)
                  .method("runtime",         &TaskResult::runtime)
                  .method("result",          &TaskResult::result);

        // register the error methods
        errorResult.method("started",        &ErrorResult::started)
                   .method("finished",       &ErrorResult::finished)
                   .method("executable",     &ErrorResult::executable)
                   .method("arguments",      &ErrorResult::arguments)
                   .method("stdin",          &ErrorResult::stdin)
                   .method("stdout",         &ErrorResult::stdout)
                   .method("stderr",         &ErrorResult::stderr);

        // register stats methods
        stats.method("first",       &Stats::first)
             .method("last",        &Stats::last)
             .method("finished",    &Stats::finished)
             .method("fastest",     &Stats::fastest)
             .method("slowest",     &Stats::slowest)
             .method("processes",   &Stats::processes)
             .method("runtime",     &Stats::runtime)
             .method("input",       &Stats::input)
             .method("output",      &Stats::output);

        // register datastats methods
        datastats.method("files",   &DataStats::files)
                 .method("bytes",   &DataStats::bytes);

        // register winner methods
                // register stats methods
        winner.method("input",    &Winner::input)
              .method("output",   &Winner::output)
              .method("error",    &Winner::error)
              .method("server",   &Winner::server)
              .method("pid",      &Winner::pid)
              .method("signal",   &Winner::signal)
              .method("exit",     &Winner::exit)
              .method("started",  &Winner::started)
              .method("finished", &Winner::finished)
              .method("runtime",  &Winner::runtime);


        // create the map reduce interface
        Php::Interface mapreduce("Yothalot\\MapReduce");

        // register the interface methods
        mapreduce.method("map", {
            Php::ByVal("record", Php::Type::Null),
            Php::ByVal("reducer", "Yothalot\\Reducer")
        }).method("reduce", {
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("values", "Yothalot\\Values"),
            Php::ByVal("writer", "Yothalot\\Writer")
        }).method("write", {
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null)
        }).method("includes");

        // create an interface for the kvmapreduce, because we cannot
        // create an interface with optional parameters so that the old mapreduce
        // keeps working as it did
        Php::Interface kvmapreduce("Yothalot\\MapReduce2");

        // register the interface methods
        kvmapreduce.method("map", {
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null),
            Php::ByVal("reducer", "Yothalot\\Reducer")
        }).method("reduce", {
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("values", "Yothalot\\Values"),
            Php::ByVal("writer", "Yothalot\\Writer")
        }).method("write", {
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null)
        }).method("includes");

        // create the race interface
        Php::Interface race("Yothalot\\Race");

        // register the interface methods
        race.method("process", {
            Php::ByVal("data", Php::Type::String)
        }).method("includes");

        // create the task interface
        Php::Interface task("Yothalot\\Task");

        // register the interface methods
        task.method("process", {
        }).method("includes");

        // move all the classes to the extension
        extension.add(std::move(writer));
        extension.add(std::move(reducer));
        extension.add(std::move(values));
        extension.add(std::move(connection));
        extension.add(std::move(job));
        extension.add(std::move(path));
        extension.add(std::move(mapreduce));
        extension.add(std::move(kvmapreduce));
        extension.add(std::move(race));
        extension.add(std::move(task));
        extension.add(std::move(input));
        extension.add(std::move(output));
        extension.add(std::move(record));
        extension.add(std::move(mapReduceResult));
        extension.add(std::move(raceResult));
        extension.add(std::move(taskResult));
        extension.add(std::move(errorResult));
        extension.add(std::move(stats));
        extension.add(std::move(datastats));
        extension.add(std::move(winner));

        // add the init method for use on the command line to our namespace, this
        // will result in `php -r "YothalotInit('mapper');"`
        extension.add("YothalotInit", &yothalotInit, { Php::ByVal("mode", Php::Type::String) });

        // add all the ini settings for the connection
        extension.add(Php::Ini{ "yothalot.host",         "localhost"    });
        extension.add(Php::Ini{ "yothalot.user",         "guest"        });
        extension.add(Php::Ini{ "yothalot.password",     "guest"        });
        extension.add(Php::Ini{ "yothalot.vhost",        "/"            });
        extension.add(Php::Ini{ "yothalot.exchange",     ""             });
        extension.add(Php::Ini{ "yothalot.mapreduce",    "mapreduce"    });
        extension.add(Php::Ini{ "yothalot.races",        "races"        });
        extension.add(Php::Ini{ "yothalot.jobs",         "jobs"         });

        // add the ini property for the base directory
        extension.add(Php::Ini("yothalot.base-directory", ""));

        // return the extension
        return extension;
    }
}
