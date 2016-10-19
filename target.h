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
 *  Class definition
 */
class Target : public Yothalot::Target
{
public:
    /**
     *  Constructor
     *  @param  cache           cache settings to use
     *  @param  directory       directory to use
     */
    Target(const std::shared_ptr<Cache> &cache, const char *directory) :
        Yothalot::Target(cache->connection(), directory, cache->maxsize(), cache->ttl()) {}

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

