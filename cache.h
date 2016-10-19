/**
 *  Cache.h
 *
 *  Class that collects all cache settings from the php.ini file and turns
 *  it into a Yothalot::Target object
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "datasize.h"

/**
 *  Class definitions
 */
class Cache
{
private:
    /**
     *  The nosql address
     *  @var std::string
     */
    std::string _address;

    /**
     *  The nosql connection
     *  @var Copernica::NoSql::Connection
     */
    Copernica::NoSql::Connection _connection;

    /**
     *  Max-cache setting
     *  @var size_t
     */
    size_t _maxsize;
    
    /**
     *  TTL setting
     *  @var time_t
     */
    time_t _ttl;

    
    /**
     *  Helper class to extract data from a php::value
     */
    class Helper
    {
    private:
        /**
         *  Reference to the php value
         *  @var Php::Value
         */
        const Php::Value &_value;
    
        /**
         *  The offset
         *  @var size_t
         */
        size_t _offset;
    
    public:
        /**
         *  Constructor
         *  @param  value
         *  @param  offset
         */
        Helper(const Php::Value &value, size_t offset) :
            _value(value), _offset(offset) {}
            
        /**
         *  Destructor
         */
        virtual ~Helper() = default;
    
        /**
         *  Expose members
         *  @return address
         */
        const char *address() const
        {
            // expose if it is set in the array
            if ((size_t)_value.size() > _offset) return _value[_offset];
            
            // use the default
            return Php::ini_get("yothalot.cache");
        }
        
        /**
         *  The maxcache setting
         *  @return size_t
         */
        size_t maxcache() const
        {
            // expose if it is set in the array
            if ((size_t)_value.size() > _offset + 1) return DataSize(_value[_offset + 1]);
            
            // use the default
            return DataSize(Php::ini_get("yothalot.maxcache"));
        }

        /**
         *  The ttl setting
         *  @return size_t
         */
        size_t ttl() const
        {
            // expose if it is set in the array
            if ((size_t)_value.size() > _offset + 2) return (int64_t)_value[_offset + 2];
            
            // use the default
            return (int64_t)Php::ini_get("yothalot.ttl");
        }
    };

    /**
     *  Constructor
     *  @param  helper
     */
    Cache(const Helper &helper) : 
        _address(helper.address()), 
        _connection(helper.address()), 
        _maxsize(helper.maxcache()), 
        _ttl(helper.ttl()) {}

public:
    /**
     *  Constructor
     *  @param  address     address of the nosql server
     *  @param  maxcache    max size of items to be cached
     *  @param  ttl         time-to-live for cached items
     */
    Cache(std::string address, size_t maxcache, time_t ttl) : 
        _address(std::move(address)), _connection(_address.data()), _maxsize(maxcache), _ttl(ttl) {}

    /**
     *  Constructor
     * 
     *  @todo this can be removed / do we use the DataSize object in all places setting?
     */
    Cache() : Cache(Php::ini_get("yothalot.cache"), DataSize(Php::ini_get("yothalot.maxcache")), (int64_t)Php::ini_get("yothalot.ttl")) {}
    
    /**
     *  Constructor
     *  @param  data        unserialized input data
     *  @param  offset      number of initial entries to skip
     */
    Cache(const Php::Value &data, size_t offset) :
        Cache(Helper(data, offset)) {}
    
    /**
     *  Destructor
     */
    virtual ~Cache() = default;
    
    /**
     *  Expose the address
     *  @return std::string
     */
    const std::string &address() const
    {
        // expose member
        return _address;
    }
    
    /**
     *  Max size for items in the cache
     *  @return size_t
     */
    size_t maxsize() const
    {
        // expose member
        return _maxsize;
    }
    
    /**
     *  Expose the ttl for items
     *  @return time_t
     */
    time_t ttl() const
    {
        // expose member
        return _ttl;
    }
    
    /**
     *  Expose the nosql connection
     *  @return Copernica::NoSql::Connection
     */
    Copernica::NoSql::Connection *connection()
    {
        return &_connection;
    }
};

