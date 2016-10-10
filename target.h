/**
 *  Target.h
 *
 *  Extended Yothalot\Target object that is simpler to construct
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
 *  Class definition
 */
class Target : public Yothalot::Target
{
public:
    /**
     *  Constructor
     *  @param  connection      nosql connection
     *  @param  directory       directory to use
     */
    Target(Copernica::NoSql::Connection *connection, const char *directory) :
        Yothalot::Target(connection, directory, DataSize(Php::ini_get("yothalot.maxcache")), (int64_t)Php::ini_get("yothalot.ttl")) {}

    /**
     *  Constructor
     *  @param  directory       directory to use
     */
    Target(const char *directory) :
        Yothalot::Target(directory) {}
        
    /**
     *  Destructor
     */
    virtual ~Target() = default;
};

