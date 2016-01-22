<?php

/**
 *  Dependencies
 */
require_once('KvChainedLineCount.php');
require_once('Merger.php');

/**
 *  This should be 3x the linecount-results by definition, because
 *  multiple inputs are being used.
 *
 *  @var Yothalot\Path
 */
$path = new Yothalot\Path("linecount-results-3.txt");

/**
 *  Unlink the result upon start, to make sure that we don't display the previous result.
 */
unlink($path->absolute());

/**
 *  Create an instance of the merger
 *  @var Merger
 */
$merger = new Merger($path->relative());

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
 *  Create the merging job
 *  @var Job
 */
$job = new Yothalot\Job($connection, $merger);

/**
 *  Create an instance of the WordCount algorithm
 *  @var WordCount
 */
$linecount1 = new KvChainedLineCount($job->directory()."/tmp1.out");
$linecount2 = new KvChainedLineCount($job->directory()."/tmp2.out");
$linecount3 = new KvChainedLineCount($job->directory()."/tmp3.out");

/**
 *  Start several subjobs
 *  @var Job
 */
$sub1 = new Yothalot\Job($connection, $linecount1);
$sub2 = new Yothalot\Job($connection, $linecount2);
$sub3 = new Yothalot\Job($connection, $linecount3);

/**
 *  Assign a job an input
 */
function assign($job, $path)
{
    $objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($path), RecursiveIteratorIterator::SELF_FIRST);
    foreach($objects as $name => $object) {

        // add the jobs with an empty key, all to singular files
        $job->add("", $object->getPathName());

    }
}

// assign all subjobs the input
assign($sub1, getcwd());
assign($sub2, getcwd());
assign($sub3, getcwd());

/**
 *  We wait for all the jobs. In the real case you'd probably want to make
 *  sure that everything went as planned, but we don't do that here
 */
$sub1->wait();
$sub2->wait();
$sub3->wait();

/**
 *  Now start the final job
 */
$job->wait();

/**
 *  Show the found words
 */
echo(file_get_contents($path->absolute()));
