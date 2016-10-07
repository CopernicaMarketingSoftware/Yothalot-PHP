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
     *  The nosql connection
     *  @var Copernica::NoSql::Connection
     */
    Copernica::NoSql::Connection _connection;

    /**
     *  The actual yothalot target class
     *  @var Yothalot::Target
     */
    Yothalot::Target _target;

public:
    /**
     *  Constructor
     */
    Cache() : 
        _connection(Php::ini_get("yothalot.cache")),
        _target(&_connection, DataSize(Php::ini_get("yothalot.maxcache")), (int64_t)Php::ini_get("yothalot.ttl")) {}
    
    /**
     *  Destructor
     */
    virtual ~Cache() = default;
    
    /**
     *  Cast to a target pointer
     *  @return Yothalot::Target*
     */
    operator Yothalot::Target* ()
    {
        // expose the target
        return &_target;
    }
};

