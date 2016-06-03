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
#include "error.h"
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
 *  The different error types
 */
using   MapReduceError  =   Error<MapReduceResult>;
using   RaceError       =   Error<RaceResult>;
using   TaskError       =   Error<TaskResult>;

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
        Php::Class<MapReduceError>  mapReduceError ("Yothalot\\MapReduceError");
        Php::Class<RaceResult>      raceResult     ("Yothalot\\RaceResult");
        Php::Class<RaceError>       raceError      ("Yothalot\\RaceError");
        Php::Class<TaskResult>      taskResult     ("Yothalot\\TaskResult");
        Php::Class<TaskError>       taskError      ("Yothalot\\TaskError");
        Php::Class<Stats>           stats          ("Yothalot\\Stats");
        Php::Class<DataStats>       datastats      ("Yothalot\\DataStats");
        Php::Class<Winner>          winner         ("Yothalot\\Winner");

        // register writer functions
        writer.method<&Writer::emit>("emit", {
            Php::ByVal("value", Php::Type::Null)
        });

        // register functions on the reducer
        reducer.method<&Reducer::emit>("emit", {
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null)
        });

        // register the Connection methods
        connection.method<&Connection::__construct>("__construct", {
            Php::ByVal("settings", Php::Type::Array, false)
        }).method<&Connection::flush>("flush");

        // register the methods on our php classes
        job.method<&Job::__construct>("__construct", {
            Php::ByVal("connection", "Yothalot\\Connection"),
            Php::ByVal("algorithm") // This should be either Yothalot\MapReduce, Yothalot\MapReduce2, Yothalot\Race or Yothalot\Task
        }).method<&Job::splitsize>("splitsize", {
            Php::ByVal("splitsize", Php::Type::Numeric)
        }).method<&Job::add>("add", { // old, v1 on single argument. new, singular keys/values otherwise for more arguments
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null, false),
            Php::ByVal("server", Php::Type::String, false)
        }).method<&Job::map>("map", { // this should ensure the multiple case, where mapped values will be combined
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null),
            Php::ByVal("server", Php::Type::String, false)
        }).method<&Job::file>("file", { // new, v2 behaviour
            Php::ByVal("filename", Php::Type::String),
            Php::ByVal("start", Php::Type::Numeric, false),
            Php::ByVal("size", Php::Type::Numeric, false),
            Php::ByVal("remove", Php::Type::Bool, false),
            Php::ByVal("server", Php::Type::String, false)
        }).method<&Job::directory>("directory", { // new, v2 behaviour
            Php::ByVal("dirname", Php::Type::String, false),
            Php::ByVal("remove", Php::Type::Bool, false),
            Php::ByVal("server", Php::Type::String, false)
        }).method<&Job::modulo>("modulo", {
            Php::ByVal("value", Php::Type::Numeric)
        }).method<&Job::maxprocesses>("maxprocesses", {
            Php::ByVal("value", Php::Type::Numeric)
        }).method<&Job::maxfiles>("maxfiles", { // old behaviour causes all the same, new behaviour has differences
            Php::ByVal("mapper", Php::Type::Numeric),
            Php::ByVal("reducer", Php::Type::Numeric, false),
            Php::ByVal("finalizer", Php::Type::Numeric, false)
        }).method<&Job::maxbytes>("maxbytes", { // old behaviour sets all values to the same value, new behaviour set individually
            Php::ByVal("mapper", Php::Type::Numeric),
            Php::ByVal("reducer", Php::Type::Numeric, false),
            Php::ByVal("finalizer", Php::Type::Numeric, false)
        }).method<&Job::maxrecords>("maxrecords", {
            Php::ByVal("mapper", Php::Type::Numeric)
        }).method<&Job::maxmappers>("maxmappers", {
            Php::ByVal("value", Php::Type::Numeric)
        }).method<&Job::maxreducers>("maxreducers", {
            Php::ByVal("value", Php::Type::Numeric)
        }).method<&Job::maxfinalizers>("maxfinalizers", {
            Php::ByVal("value", Php::Type::Numeric)
        }).method<&Job::local>("local", {
            Php::ByVal("value", Php::Type::Bool)
        }).method<&Job::flush>("flush", { // new, v2 behaviour
        }).method<&Job::start>("start", {
        }).method<&Job::detach>("detach", {
        }).method<&Job::wait>("wait");

        // register the path methods
        path.method<&Path::__construct>("__construct", {
            Php::ByVal("path", Php::Type::String)
        }).method<&Path::absolute>("absolute", {
        }).method<&Path::relative>("relative");

        // register the output methods
        output.method<&Output::__construct>("__construct", {
            Php::ByVal("filename", Php::Type::String),
            Php::ByVal("splitsize", Php::Type::Numeric, false)
        }).method<&Output::add>("add", {
            Php::ByVal("identifier", Php::Type::Numeric),
            Php::ByVal("fields", Php::Type::Array)
        }).method<&Output::kv>("kv", {
            Php::ByVal("key", Php::Type::Null),
            Php::ByVal("value", Php::Type::Null)
        }).method<&Output::name>("name", {
        }).method<&Output::flush>("flush", {
            Php::ByVal("recompress", Php::Type::Bool, false)
        }).method<&Output::size>("size");

        // register input methods
        input.method<&Input::__construct>("__construct", {
            Php::ByVal("filename", Php::Type::String),
            Php::ByVal("strict", Php::Type::Bool, false)
        }).method<&Input::name>("name", {
        }).method<&Input::size>("size")
          .method<&Input::valid>("valid")
          .method<&Input::next>("next")
          .method<&Input::seek>("seek");

        // register record methods
        record.method<&Record::identifier>("identifier", {
        }).method<&Record::size>("size", {
        }).method<&Record::fields>("count", {
        }).method<&Record::array>("array", {
        });

        // make a simple Result Interface that only we are implementing..
        Php::Interface result("Yothalot\\Result");

        // register the common methods
        result  .method("started",  {})
                .method("finished", {})
                .method("runtime",  {});

        // register map reduce result methods
        mapReduceResult.implements(result)
                       .method<&MapReduceResult::started>       ("started")
                       .method<&MapReduceResult::finished>      ("finished")
                       .method<&MapReduceResult::runtime>       ("runtime")
                       .method<&MapReduceResult::mappers>       ("mappers")
                       .method<&MapReduceResult::reducers>      ("reducers")
                       .method<&MapReduceResult::finalizers>    ("finalizers");

        // and the error result for map/reduce
        mapReduceError  .extends(mapReduceResult)
                        .method<&MapReduceError::executable>    ("executable")
                        .method<&MapReduceError::arguments>     ("arguments")
                        .method<&MapReduceError::stdin>         ("stdin")
                        .method<&MapReduceError::stdout>        ("stdout")
                        .method<&MapReduceError::stderr>        ("stderr")
                        .method<&MapReduceError::command>       ("command");

        // register the race result methods
        raceResult.implements(result)
                  .method<&RaceResult::started>     ("started")
                  .method<&RaceResult::finished>    ("finished")
                  .method<&RaceResult::runtime>     ("runtime")
                  .method<&RaceResult::processes>   ("processes")
                  .method<&RaceResult::result>      ("result")
                  .method<&RaceResult::winner>      ("winner");

        // and the error result for race jobs
        raceError   .extends(raceResult)
                    .method<&RaceError::executable> ("executable")
                    .method<&RaceError::arguments>  ("arguments")
                    .method<&RaceError::stdin>      ("stdin")
                    .method<&RaceError::stdout>     ("stdout")
                    .method<&RaceError::stderr>     ("stderr")
                    .method<&RaceError::command>    ("command");

        // register the task result methods
        taskResult.implements(result)
                  .method<&TaskResult::started>     ("started")
                  .method<&TaskResult::finished>    ("finished")
                  .method<&TaskResult::runtime>     ("runtime")
                  .method<&TaskResult::result>      ("result");

        // and the error result for race jobs
        taskError   .extends(raceResult)
                    .method<&TaskError::executable> ("executable")
                    .method<&TaskError::arguments>  ("arguments")
                    .method<&TaskError::stdin>      ("stdin")
                    .method<&TaskError::stdout>     ("stdout")
                    .method<&TaskError::stderr>     ("stderr")
                    .method<&TaskError::command>    ("command");

        // register stats methods
        stats.method<&Stats::first>     ("first")
             .method<&Stats::last>      ("last")
             .method<&Stats::finished>  ("finished")
             .method<&Stats::fastest>   ("fastest")
             .method<&Stats::slowest>   ("slowest")
             .method<&Stats::processes> ("processes")
             .method<&Stats::runtime>   ("runtime")
             .method<&Stats::input>     ("input")
             .method<&Stats::output>    ("output");

        // register datastats methods
        datastats.method<&DataStats::files> ("files")
                 .method<&DataStats::bytes> ("bytes");

        // register winner methods
        winner.method<&Winner::input>   ("input")
              .method<&Winner::output>  ("output")
              .method<&Winner::error>   ("error")
              .method<&Winner::server>  ("server")
              .method<&Winner::pid>     ("pid")
              .method<&Winner::signal>  ("signal")
              .method<&Winner::exit>    ("exit")
              .method<&Winner::started> ("started")
              .method<&Winner::finished>("finished")
              .method<&Winner::runtime> ("runtime");


        // create the map reduce interface
        Php::Interface mapreduce("Yothalot\\MapReduce");

        // register the interface methods
        mapreduce.method("map", {
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

        // we alias MapReduce2 to MapReduce
        Php::Interface mapreduce2("Yothalot\\MapReduce2");
        mapreduce2.extends(mapreduce);

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
        extension.add(std::move(mapreduce2));
        extension.add(std::move(race));
        extension.add(std::move(task));
        extension.add(std::move(input));
        extension.add(std::move(output));
        extension.add(std::move(record));
        extension.add(std::move(result));
        extension.add(std::move(mapReduceResult));
        extension.add(std::move(mapReduceError));
        extension.add(std::move(raceResult));
        extension.add(std::move(raceError));
        extension.add(std::move(taskResult));
        extension.add(std::move(taskError));
        extension.add(std::move(stats));
        extension.add(std::move(datastats));
        extension.add(std::move(winner));

        // add the init method for use on the command line to our namespace, this
        // will result in `php -r "YothalotInit('mapper');"`
        extension.add<yothalotInit>("YothalotInit", { Php::ByVal("mode", Php::Type::String) });

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
