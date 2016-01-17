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
require_once('KvLineCount.php');

/**
 *  The WordCount class wrote its output to a file on the distributed file
 *  system. To find out what the absolute path name of this file is on this
 *  machine, we make use of the Yothalot\Path class to turn the relative name
 *  into an absolute path (GlusterFS must be mounted on this machine)
 *
 *  @var Yothalot\Path
 */
$path = new Yothalot\Path("linecount-results-kv.txt");

/**
 *  Unlink the result upon start, to make sure that we don't display the previous result.
 */
unlink($path->absolute());

/**
 *  Create an instance of the WordCount algorithm
 *  @var WordCount
 */
$wordcount = new KvLineCount($path->relative());

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

function assign($job, $path)
{
    $objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($path), RecursiveIteratorIterator::SELF_FIRST);
    foreach($objects as $name => $object) {

        // add the jobs with an empty key, all to singular files
        $job->add("", $object->getPathName());

    }
}

assign($job, getcwd());

/**
 *  Wait for the result of the map reduce job
 */
$result = $job->wait();

/**
 *  Show the found words
 */
echo(file_get_contents($path->absolute()));
