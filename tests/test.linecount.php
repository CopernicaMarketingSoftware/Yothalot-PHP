<?php
/**
 *  Script to test
 *
 *  @copyright 2015 Copernica BV
 *  @documentation private
 */

/**
 *  Dependencies
 */
require_once('LineCount.php');

/**
 *  The output file on the gluster
 *  @var string
 */
$output = "linecount-results.txt";

/**
 *  Create an instance of the WordCount algorithm
 *  @var WordCount
 */
$wordcount = new LineCount($output);

/**
 *  We want to send this WordCount instance to the Yothalot master. To do this,
 *  we need an instance of this master object.
 *
 *  (Under the hood, you do not connect with the Yothalot master process, but to
 *  a RabbitMQ message queue, the login details are therefore the RabbitMQ
 *  details)
 *
 *  @var Yothalot\Master
 */
$master = new Yothalot\Connection();

/**
 *  Now that we have access to the master, we can tell the master to create a
 *  new MapReduce job, using our WordCount implementation. The return value
 *  is a Yothalot\Job object, that has many methods to feed data to the job,
 *  and to finetune the job.
 *
 *  @var Yothalot\Job
 */
$job = new Yothalot\Job($master, $wordcount);

/**
 *  Function to list all files and add the pathname
 *
 *  @param  job     The job to add it to
 *  @param  path    The path to search
 */
function assign($job, $path)
{
    $objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($path), RecursiveIteratorIterator::SELF_FIRST);
    foreach($objects as $name => $object) {

        $job->add($object->getPathName());
    }
}

/**
 *  Add every file in the current working directory to the job
 */
assign($job, getcwd());

/**
 *  Start the job, from which point we can no longer add any data.
 */
$job->start();

/**
 *  Wait for the result of the map reduce job
 */
$job->wait();

/**
 *  The WordCount class wrote its output to a file on the distributed file
 *  system. To find out what the absolute path name of this file is on this
 *  machine, we  make use of the Yothalot\Path class to turn the relative name
 *  into an absolute path (GlusterFS must be mounted on this machine)
 *
 *  @var Yothalot\Path
 */
$path = new Yothalot\Path($output);

/**
 *  Show the found words
 */
echo(file_get_contents($path->absolute()));
