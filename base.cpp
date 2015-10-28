/**
 *  Base.cpp
 *
 *  Implementation file for retrieving GlusterFS mount point
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include "base.h"
#include <phpcpp.h>

/**
 *  Function returns the base directory
 *  @return Yothalot::Base
 */
const Yothalot::Base &base()
{
    // create static instance
    static Yothalot::Base instance(Php::ini_get("yothalot.base-directory"));

    // return it
    return instance;
}
