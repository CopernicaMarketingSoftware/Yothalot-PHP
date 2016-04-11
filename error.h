/**
 *  error.h
 *
 *  PHP class for an operation resulting in an error, it
 *  extends the result class for the requested operation
 *
 *  @copyright 2016 Copernica B.V.
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Class definition
 */
template <class Parent>
class Error : public Parent
{
private:
    /**
     *  Retrieve the relevant JSON
     *
     *  @return The JSON containing the error information
     */
    JSON::Object json() const
    {
        // retrieve the error data
        return Parent::_json.object("error");
    }

public:
    /**
     *  Constructor
     *
     *  @param  json    The json containing the result of the operation
     */
    Error(const JSON::Object &json) : Parent(json) {}

    /**
     *  Destructor
     */
    virtual ~Error() = default;

    /**
     *  We're an error, we evaluate to false
     *
     *  @return Does the result indicate a success?
     */
    Php::Value __toBool() const
    {
        // this is not a success
        return false;
    }

    /**
     *  Get the exectuable that was used for this job
     *  @return Php::Value
     */
    Php::Value executable() const
    {
        return json().c_str("executable");
    }

    /**
     *  Get the arguments that was used for this job
     *  @return Php::Value
     */
    Php::Value arguments() const
    {
        // get the arguments
        auto args = json().array("arguments");

        // return as a php value
        return args.phpValue();
    }

    /**
     *  Return the stdin that was passed to this job
     *  @return Php::Value
     */
    Php::Value stdin() const
    {
        return json().c_str("stdin");
    }

    /**
     *  Return the stdout that was passed to this job
     *  @return Php::Value
     */
    Php::Value stdout() const
    {
        return json().c_str("stdout");
    }

    /**
     *  Return the stderr that was passed to this job
     *  @return Php::Value
     */
    Php::Value stderr() const
    {
        return json().c_str("stderr");
    }

    /**
     *  The command to execute to re-run the script again
     *
     *  @return The complete line to execute to reproduce the error
     */
    Php::Value command() const
    {
        // first build it up inside a string
        std::string command;

        // retrieve the arguments
        auto arguments = json().array("arguments");

        // we need to echo the input and pipe it
        command.append("echo ");
        command.append(Php::call("escapeshellarg", json().c_str("stdin")).stringValue());
        command.append(" | ");

        // now we need the actual command
        command.append(json().c_str("executable"));

        // iterate over the arguments
        for (int i = 0; i < arguments.size(); ++i)
        {
            // add the separator and argument
            command.push_back(' ');
            command.append(Php::call("escapeshellarg", arguments.c_str(i)).stringValue());
        }

        // return the completed command
        return command;
    }
};
