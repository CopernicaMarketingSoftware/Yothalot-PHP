<?php
/**
 *  Dependencies
 */
require_once('WordCount.php');

/**
 *  The WordCount class wrote its output to a file on the distributed file
 *  system. To find out what the absolute path name of this file is on this
 *  machine, we make use of the Yothalot\Path class to turn the relative name
 *  into an absolute path (GlusterFS must be mounted on this machine)
 *
 *  @var Yothalot\Path
 */
$path = new Yothalot\Path("wordcount-results.txt");

/**
 *  Unlink the result upon start, to make sure that we don't display the previous result.
 */
unlink($path->absolute());

/**
 *  Create an instance of the WordCount algorithm
 *  @var WordCount
 */
$wordcount = new WordCount($path->relative());

/**
 *  We want to send this WordCount instance to the Yothalot connection. To do this,
 *  we need an instance of the connection to Yothalot.
 *
 *  (Under the hood, you do not connect with the Yothalot master process, but to
 *  a RabbitMQ message queue, the login details are therefore the RabbitMQ
 *  details)
 *
 *  @var Yothalot\Connect
 */
$connection = new Yothalot\Connection();

/**
 *  Now that we have access to a connection, we can create a
 *  new MapReduce job object, using this connection and our WordCount
 * implementation. The job object has many methods to feed data to the job,
 *  and to fine tune the job.
 *
 *  @var Yothalot\Job
 */
$job = new Yothalot\Job($connection,$wordcount);

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
 *  Start the job. After starting the job, no extra data can be added to job anymore.
 */
$job->start();

/**
 *  Wait for the job to be ready (this could take some time because the Yothalot
 *  master has to start up various mapper, reducer and writer processes to
 *  run the job)
 */
$job->wait();

/**
 *  Show the found words
 */
echo(file_get_contents($path->absolute()));
?>
