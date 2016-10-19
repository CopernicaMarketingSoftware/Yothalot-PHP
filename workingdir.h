/**
 *  WorkingDir.h
 *
 *  Class to temporarily change the working dir
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
class WorkingDir
{
private:
    /**
     *  The old working dir
     *  @var char *
     */
    char *_olddir;

public:
    /**
     *  Constructor
     *  @param  directory       the directory to change the current-working-dir to
     */
    WorkingDir(const char *directory) : _olddir(get_current_dir_name())
    {
        // change directory
        if (chdir(directory) == 0) return;
        
        // failed to change dir, deallocate already allocated resources
        free(_olddir);
        
        // throw an exception
        throw std::runtime_error(strerror(errno));
    }
    
    /**
     *  Destructor
     */
    virtual ~WorkingDir()
    {
        // change back to the old dir
        if (chdir(_olddir) != 0) Php::warning << "failed to change back to " << _olddir;
        
        // free resources
        free(_olddir);
    }
};


