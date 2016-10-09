/**
 *  Directory.h
 *
 *  Creates a temporary directory in a folder where the Yothalot data
 *  files are going to be stored
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "base.h"
#include <string>
#include <phpcpp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>

/**
 *  Class definition
 */
class Directory
{
private:
    /**
     *  The full name of the directory
     *  @var    Yothalot::Fullname
     */
    Yothalot::Fullname _name;
    
    /**
     *  Entry pointer
     *  @var struct dirent *
     */
    mutable struct dirent *_entry = nullptr;

public:
    /**
     *  Constructor, will create a temporary directory in the gluster mount point/tmp
     */
    Directory() : _name(base(), std::string("tmp/") + (std::string)Yothalot::UniqueName())
    {
        // Calculate buffer size (based on advise in man page)
        long name_max = pathconf(_name.full(), _PC_NAME_MAX);

        // If limit is not defined or something went wrong, we guess the max size
        if (name_max == -1) name_max = 511;

        // calculate full max size of the struct dirent
        size_t len = offsetof(struct dirent, d_name) + name_max + 1;

        // Allocate the buffer
        _entry = static_cast<struct dirent*>(malloc(len));
    }

    /**
     *  Constructor on string, that will use temporary directory in the gluster mount point/tmp
     *  @param  name        name of the directory, relative to the glusterfs mount point (should not be null)
     */
    Directory(const char *name) : _name(base(), name) {}

    /**
     *  Destructor, can be empty since the server is cleaning up this directory
     */
    virtual ~Directory() 
    {
        // clean up entry resource
        free(_entry);
    }
    
    /**
     *  Cast to boolean: does the directory exist?
     *  @return bool
     */
    operator bool () const { return exists(); }
    bool operator! () const { return !exists(); }
    
    /**
     *  Does the directory exist?
     *  @return bool
     */
    bool exists() const
    {
        // file properties
        struct stat props;
        
        // read in properties
        if (stat(_name.full(), &props) != 0) return false;
        
        // must be a dir
        return S_ISDIR(props.st_mode);
    }

    /**
     *  Create the directory
     *  @return bool
     */
    bool create()
    {
        // try to construct the directory
        if (mkdir(_name.full(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0) return true;

        // directory could not be created, make sure base exists
        Yothalot::Fullname basedir(base(), "tmp");

        // create the base directory (we don't check the result, it can also fail
        // because it already exists, and we do not want to check all such details)
        mkdir(basedir.full(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        // retry to create the actual directory
        return mkdir(_name.full(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
    }

    /**
     *  Get to the full path
     *  @return const char *
     */
    const char *full() const
    {
        return _name.full();
    }

    /**
     *  Get the relative path, relative to the gluster mount point.
     *  @return const std::string
     */
    const char *relative() const
    {
        // delegate
        return _name.relative();
    }
    
    /**
     *  Traverse the directory and apply a callback to each item
     *  passing the raw entry information
     *  it resets at the end so it can be called again.
     *  @param  callback
     *  @return bool
     */
    bool traverse(const std::function<void(struct dirent *entry)> &callback) const
    {
        // temporary result variable
        struct dirent *result = nullptr; 

        // reset the dir so traverse can be called again.
        auto *dirp = opendir(_name);

        // leap out if directory could not be opened
        if (!dirp) return false;
        
        // iterate over the files and stop if all files are consumed
        // readdir_r will return a !0 if there occurs an error
        while(!readdir_r(dirp, _entry, &result))
        {
            // If all files are consumed we break
            if (result == nullptr) break;

            // we ignore the '.' and '..' files
            if (strcmp(result->d_name, ".") == 0 || strcmp(result->d_name, "..") == 0) continue;

            // Callback
            callback(result);
        }

        // close dir
        closedir(dirp);
        
        // done
        return true;
    }

    /**
     *  Traverse the directory and apply a callback to each item
     *  passing the entry name as a string
     *  it resets at the end so it can be called again.
     *  @param  callback
     *  @return bool
     */
    bool traverse(const std::function<void(const char *name)> &callback) const
    {
        // call the other traverse implementation
        return traverse([callback](struct dirent *entry) {
            callback(entry->d_name);
        });
    }
    
    /**
     *  Remove the directory (and all files in it)
     *  @return true
     */
    bool remove()
    {
        // traverse over the files
        traverse([this](const char *name) {
            
            // construct full path name
            std::string fullname(_name.full());
            
            // append file name
            fullname.append("/").append(name);
            
            // remove it
            unlink(fullname.data());
        });
        
        // remove the directory
        return rmdir(full()) == 0;
    }
};

