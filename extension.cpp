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

        // start off by creating the Yothalot namespace
        Php::Namespace ns("Yothalot");

        // create the classes
        Php::Class<Writer>          writer         ("Writer");
        Php::Class<Reducer>         reducer        ("Reducer");
        Php::Class<Values>          values         ("Values");
        Php::Class<Connection>      connection     ("Connection");
        Php::Class<Job>             job            ("Job");
        Php::Class<Path>            path           ("Path");
        Php::Class<Output>          output         ("Output");
        Php::Class<Input>           input          ("Input");
        Php::Class<Record>          record         ("Record");
        Php::Class<MapReduceResult> mapReduceResult("MapReduceResult");
        Php::Class<RaceResult>      raceResult     ("RaceResult");
        Php::Class<Stats>           stats          ("Stats");
        Php::Class<DataStats>       datastats      ("DataStats");
        Php::Class<Winner>          winner         ("Winner");

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
            Php::ByVal("algorithm") // This should be either Yothalot\MapReduce or Yothalot\Race
        }).method("add", &Job::add, {
            Php::ByVal("data", Php::Type::String)
        }).method("file", &Job::file, {
            Php::ByVal("data", Php::Type::String),
            Php::ByVal("filename", Php::Type::String, false)
        }).method("server", &Job::server, {
            Php::ByVal("data", Php::Type::String),
            Php::ByVal("server", Php::Type::String, false)
        }).method("modulo", &Job::modulo, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxprocesses", &Job::maxprocesses, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxfiles", &Job::maxfiles, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxbytes", &Job::maxbytes, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxmappers", &Job::maxmappers, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxreducers", &Job::maxreducers, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("maxfinalizers", &Job::maxfinalizers, {
            Php::ByVal("value", Php::Type::Numeric)
        }).method("local", &Job::local, {
            Php::ByVal("value", Php::Type::Bool)
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
        }).method("name", &Output::name, {
        }).method("flush", &Output::flush, {
        }).method("size", &Output::size);

        // register input methods
        input.method("__construct", &Input::__construct, {
            Php::ByVal("filename", Php::Type::String),
            Php::ByVal("start", Php::Type::Numeric, false),
            Php::ByVal("bytes", Php::Type::Numeric, false)
        }).method("name", &Input::name, {
        }).method("size", &Input::size);

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

        raceResult.method("started",         &RaceResult::started)
                  .method("finished",        &RaceResult::finished)
                  .method("runtime",         &RaceResult::runtime)
                  .method("processes",       &RaceResult::processes)
                  .method("result",          &RaceResult::result)
                  .method("winner",          &RaceResult::winner);
                  

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
        Php::Interface mapreduce("MapReduce");

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

        // create the race interface
        Php::Interface race("Race");

        // register the interface methods
        race.method("process", {
            Php::ByVal("data", Php::Type::String)
        }).method("includes");

        // move all the classes to the namespace
        ns.add(std::move(writer));
        ns.add(std::move(reducer));
        ns.add(std::move(values));
        ns.add(std::move(connection));
        ns.add(std::move(job));
        ns.add(std::move(path));
        ns.add(std::move(mapreduce));
        ns.add(std::move(race));
        ns.add(std::move(input));
        ns.add(std::move(output));
        ns.add(std::move(record));
        ns.add(std::move(mapReduceResult));
        ns.add(std::move(raceResult));
        ns.add(std::move(stats));
        ns.add(std::move(datastats));
        ns.add(std::move(winner));

        // add the init method for use on the command line to our namespace, this
        // will result in `php -r "YothalotInit('mapper');"`
        extension.add("YothalotInit", &yothalotInit, { Php::ByVal("mode", Php::Type::String) });

        // add the ini property for the base directory
        extension.add(Php::Ini("yothalot.base-directory", ""));

        // add the classes to the extension
        extension.add(std::move(ns));

        // return the extension
        return extension;
    }
}
