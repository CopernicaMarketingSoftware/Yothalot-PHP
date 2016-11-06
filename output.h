/**
 *  Output.h
 *
 *  Utility class for writing files in the same format as the intermediate files
 *  that are used by the mapreduce algorithm
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Class definition
 */
class Output : public Php::Base
{
private:
    /**
     *  The actual implementation class
     *  @var std::unique_ptr<Yothalot::Output>
     */
    std::unique_ptr<Yothalot::Output> _impl;

    /**
     *  Save the name for if we have to reconstruct ourselves later on
     *  @var std::string
     */
    std::string _name;


public:
    /**
     *  The PHP constructor
     *  @param  params
     */
    void __construct(Php::Parameters &params)
    {
        // check number of params
        if (params.size() == 0) Php::error << "No filename passed to Yothalot\\Output constructor";

        // read the params
        _name = params[0].stringValue();

        // prevent exceptions (C++ errors should not bubble up to PHP space)
        try
        {
            // construct the object based on the amount of php parameters that were passed
            _impl.reset(new Yothalot::Output(_name.data()));
        }
        catch (const std::runtime_error &error)
        {
            // turn the error into a PHP error
            throw Php::Exception(error.what());
        }
    }

    /**
     *  The PHP destructor
     */
    void __destruct()
    {
        // reset impl
        _impl = nullptr;
    }

    /**
     *  Retrieve the full file name
     *  @return Php::Value
     */
    Php::Value name() const
    {
        // pass on to imple
        return _impl->name();
    }

    /**
     *  File size
     *  @return Php::Value
     */
    Php::Value size() const
    {
        // pass on to impl
        return (int64_t)_impl->size();
    }

    /**
     *  Flush the output log
     *  @return PHP value wrapping the same object, for chaining
     */
    Php::Value flush()
    {
        // flush the file, optionally recompressing it
        _impl->flush();

        // allow chaining
        return this;
    }

    /**
     *  Add a record to the file
     *  @param  params
     *  @return Php::Value
     */
    Php::Value add(Php::Parameters &params)
    {
        // need two parameters
        if (params.size() != 2) Php::error << "Yothalot\\Output::add() requires two parameters" << std::flush;

        // get identifier and parameters
        auto identifier = params[0];
        auto values = params[1];

        // construct the record
        Yothalot::Record record(identifier.numericValue());

        // values should be an array
        if (!values.isArray()) Php::error << "Only arrays of scalar values can be added to Yothalot output files" << std::flush;

        // iterate over the values
        for (auto i = 0; i < values.size(); ++i)
        {
            // get the field
            Php::Value field = values[i];

            // check the field type
            switch (field.type()) {
            case Php::Type::Numeric:    record.add(field.numericValue()); break;
            case Php::Type::String:     record.add(field.rawValue()); break;
            case Php::Type::Null:       record.add(nullptr); break;
            default:                    Php::error << "Only integers, strings and NULL values are supported in Yothalot files" << std::flush;
            }
        }

        // add the record to the file
        _impl->add(record);

        // allow chaining
        return this;
    }

    /**
     *  Add a key/value pair to the file. This can be re-opened by the yothalot cluster.
     *  @param  params
     *  @return Php::Value
     */
    Php::Value kv(Php::Parameters &params)
    {
        // need two parameters
        if (params.size() != 2) Php::error << "Yothalot\\Output::kv($key, $value) requires two parameters" << std::flush;

        // wrap the key and value
        Tuple::Yothalot key(params[0]);
        Tuple::Yothalot value(params[1]);

        // construct the record from the keyvalue
        Yothalot::Record record(Yothalot::KeyValue(key, value));

        // add the record to the file
        _impl->add(record);

        // allow chaining
        return this;
    }
};
