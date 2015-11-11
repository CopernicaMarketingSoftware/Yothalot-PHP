/**
 *  Stdin.h
 *
 *  Input that is read from stdin, and that contains the data for the
 *  task, and the original serialized object
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Class definition
 */
class Stdin
{
private:
    /**
     *  All the data
     *  @var std::string
     */
    std::string _data;

    /**
     *  The unserialized PHP value
     *  @var Php::Value
     */
    Php::Value _object;
    
    /**
     *  The rest of the input data
     *  @var const char *
     */
    const char *_rest;

public:
    /**
     *  Constructor that reads input and turns it into a 
     */
    Stdin()
    {
        // see http://stackoverflow.com/questions/201992/how-to-read-until-eof-from-cin-in-c
        // don't skip the whitespace while reading
        std::cin >> std::noskipws;

        // use stream iterators to copy the stream to a string
        std::istream_iterator<char> it(std::cin);
        std::istream_iterator<char> end;

        // create the string
        _data.assign(it, end);
        
        // look for the \n\n separator
        auto separator = _data.find("\n\n");
        
        // should exist
        if (separator == std::string::npos) throw std::runtime_error("missing separator between serialized data and input data");
        
        // we now know where the rest of the data starts
        _rest = _data.data() + separator + 2;

        // unserialize the first part of the stdin
        Php::Value unserialized(Php::call("unserialize", Php::call("base64_decode", Php::Value(_data.data(), separator))));

        // store the includes and the actual object
        Php::Value includes = unserialized[0];
        Php::Value serializedObject = unserialized[1];

        // the return value of the includes method could be a single string
        if (includes.isString())
        {
            // include the PHP file (this could cause a PHP fatal error)
            if (!Php::include_once(includes.stringValue())) Php::error << "Failed to include " << includes.stringValue() << std::flush;
        }
        else if (includes.isArray())
        {
            // loop over all our includes and call require_once with all of them
            for (int i = 0; i < includes.size(); ++i)
            {
                // the file to include
                Php::Value path = includes[i];

                // include the PHP file (this could cause a PHP fatal error)
                if (!Php::include_once(path.stringValue())) Php::error << "Failed to include " << path << std::flush;
            }
        }

        // unserialize the inner object
        _object = Php::call("unserialize", serializedObject);
    }
    
    /**
     *  Destructor
     */
    virtual ~Stdin() = default;
    
    /**
     *  The user-supplied PHP object
     *  @return Php::Value
     */
    const Php::Value &object() const
    {
        return _object;
    }
    
    /**
     *  The input data
     *  @return const char *
     */
    const char *data() const
    {
        return _rest;
    }
    
    /**
     *  Size of the data
     *  @return size_t
     */
    size_t size() const
    {
        return _data.size() - (_rest - _data.data());
    }
};

