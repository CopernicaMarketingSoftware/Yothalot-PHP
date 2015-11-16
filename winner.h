/**
 *  Winner.h
 *
 *  Statistics of the winner of the race job
 *
 *  @author Aljar Meesters <aljar.meesters@copernica.com>
 *  @copyright Copernica BV 2015
 */
 
/**
 *  include guard
 */
#pragma once

/**
 *  dependencies
 */
#include <phpcpp.h>
#include "json/object.h"

/**
 *  class definition
 */
class Winner : public Php::Base
{
private:
    /**
     *  json that holds the information
     *  @var JSON::Object
     */
    JSON::Object _json;

public:
    /**
     *  Constructor
     *  @param json
     */
    Winner(const JSON::Object &json) : _json(json) {}

    /**
     *  input that was send to the winner
     *  @return Php::Value
     */
    Php::Value input() const
    {
        // Unserialize the result
        auto completeObject = std::string(_json.c_str("stdin"));
        
        // look for the \n\n separator
        auto separator = completeObject.find("\n\n");
        
        // should exist
        if (separator == std::string::npos) throw std::runtime_error("missing separator between serialized data and input data");
        
        // we now know where the rest of the data starts
        auto data = completeObject.data() + separator + 2;
        
        // Decode the data
        return Php::call("base64_decode" , Php::Value(data)); 

        // unserialize the first part of the stdin
        // return unserialized(Php::call("unserialize", Php::call("base64_decode", Php::Value(rest.data()))));
    }
    
    /**
     *  output that was send to stdout by winner
     *  @return Php::Value
     */
    Php::Value output() const
    {
        // Unserialize the result
        return Php::call("unserialize", Php::call("base64_decode", _json.c_str("stdout")));
        
    }
     
    /**
     *  error that was send to stderr by winner
     *  @return Php::Value
     */
    Php::Value error() const
    {
        return Php::call("unserialize", Php::call("base64_decode", _json.c_str("stderr")));
    }
    
    /**
     *  name of sever on which the winning job ran
     *  @return Php::Value
     */
    Php::Value server() const
    {
        return _json.c_str("server");
    }
     
    /**
     *  Process id of the winner
     *  @return Php::Value
     */
    Php::Value pid() const
    {
        return _json.integer("pid");
    }
    
    /**
     *  Signal if the winner was killed
     *  @return Php::Value
     */
    Php::Value signal() const
    {
        return _json.integer("signal");
    }
    
    /**
     *  Exit code with which the winner exited
     *  @return Php::Value
     */
    Php::Value exit() const
    {
        return _json.integer("exit");
    }
    
    /**
     *  Starting time of the winner
     *  @return Php::Value
     */
    Php::Value started() const
    {
        return _json.decimal("started");
    }
    
    /**
     *  Finishing time of the winner
     *  @return Php::Value
     */
    Php::Value finished() const
    {
        return _json.decimal("finished");
    }
    
    /**
     *  Runtime of the winner
     *  @return Php::Value
     */
    Php::Value runtime() const
    {
        return _json.decimal("runtime");
    }
};
