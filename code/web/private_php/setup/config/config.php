<?php
/**
* This file contains all variables needed by other php scripts.
* @author Daan Janssens, mentored by Matthew Lagoe
*/

// Variables for database access to the www/CMS database (only if support role)
$cfg['db']['web']['host']    = '%amsSqlHostname%';
$cfg['db']['web']['port']    = '3306';
$cfg['db']['web']['name']    = '%amsDatabase%';
$cfg['db']['web']['user']    = '%amsSqlUsername%';
$cfg['db']['web']['pass']    = '%amsSqlPassword%';

// Variables for database access to the lib database (only if support role)
$cfg['db']['lib']['host']    = '%amsSqlHostname%';
$cfg['db']['lib']['port']    = '3306';
$cfg['db']['lib']['name']    = '%amsLibDatabase%';
$cfg['db']['lib']['user']    = '%amsSqlUsername%';
$cfg['db']['lib']['pass']    = '%amsSqlPassword%';

// Variables for database access to the shard database
$cfg['db']['shard']['host']    = '%nelSqlHostname%';
$cfg['db']['shard']['port']    = '3306';
$cfg['db']['shard']['name']    = '%nelDatabase%';
$cfg['db']['shard']['user']    = '%nelSqlUsername%';
$cfg['db']['shard']['pass']    = '%nelSqlPassword%';

// Variables for database access to the ring_open database (only if domain role)
// $cfg['db']['ring']['host']    = '%nelSqlHostname%';
// $cfg['db']['ring']['port']    = '3306';
// $cfg['db']['ring']['name']    = '%domainDatabase%';
// $cfg['db']['ring']['user']    = '%nelSqlUsername%';
// $cfg['db']['ring']['pass']    = '%nelSqlPassword%';

// Variables for database access to the nel_tool database (only if service role)
$cfg['db']['tool']['host']    = '%nelSqlHostname%';
$cfg['db']['tool']['port']    = '3306';
$cfg['db']['tool']['name']    = '%toolDatabase%';
$cfg['db']['tool']['user']    = '%nelSqlUsername%';
$cfg['db']['tool']['pass']    = '%nelSqlPassword%';

// To connect to an IMAP server running on port 143 on the local machine,
// do the following: $mbox = imap_open("{localhost:143}INBOX", "user_id", "password");
// POP3 server on port 110: $mbox = imap_open ("{localhost:110/pop3}INBOX", "user_id", "password");
// SSL IMAP or POP3 server, add /ssl after the protocol:  $mbox = imap_open ("{localhost:993/imap/ssl}INBOX", "user_id", "password");
// To connect to an SSL IMAP or POP3 server with a self-signed certificate,
// add /ssl/novalidate-cert after the protocol specification:
// $mbox = imap_open ("{localhost:995/pop3/ssl/novalidate-cert}", "user_id", "password");
// NNTP server on port 119 use: $nntp = imap_open ("{localhost:119/nntp}comp.test", "", "");
// To connect to a remote server replace "localhost" with the name or the IP address of the server you want to connect to.
//$cfg['mail']['server'] = '{localhost:110/pop3/novalidate-cert}INBOX';

//imap connection string as explained above
$cfg['mail']['default_mailserver']= '{imap.gmail.com:993/imap/ssl}INBOX';
//groupemail is the email that sends the email
$cfg['mail']['default_groupemail'] = 'example@gmail.com';
//groupname will be the name displayed as sender
$cfg['mail']['default_groupname'] = 'Ryzom Core Support';
//the username of the account
$cfg['mail']['default_username'] = 'example@gmail.com';
//the matching password
$cfg['mail']['default_password'] = 'passw0rd';
//the host, being used when a mail is sent from a support group: support_groups_name@host
$cfg['mail']['host'] = "ryzomcore.com";

//Defines mailing related stuff
$SUPPORT_GROUP_IMAP_CRYPTKEY = "azerty";
$TICKET_MAILING_SUPPORT = false;

//You have to create this dir at first!
//The incoming emails will be backed up here and the log file keeps track of the mail_cron job.
$MAIL_DIR = "/home/username/mail";
$MAIL_LOG_PATH = "/home/username/mail/cron_mail.log";

//terms of service url location
$TOS_URL ="http://www.gnu.org/licenses/agpl-3.0.html";

//crypt is being used by encrypting & decrypting of the IMAP password of the supportgroups
$cfg['crypt']['key'] = 'Sup3rS3cr3tStuff';
$cfg['crypt']['enc_method'] = 'AES-256-CBC';
$cfg['crypt']['hash_method'] = "SHA512";

//-----------------------------------------------------------------------------------------
// If true= the server will add automatically unknown user in the database
// (in nel.user= nel.permission= ring.ring_user and ring.characters
$ALLOW_UNKNOWN = true ;
// if true= the login service automaticaly create a ring user and a editor character if needed
$CREATE_RING = true ;

// PHP server paths
$PUBLIC_PHP_PATH = "%publicPhpDirectory%";
$PRIVATE_PHP_PATH = "%privatePhpDirectory%";

// Site paths definitions (you shouldn't have to edit these..)
$AMS_LIB = $PRIVATE_PHP_PATH . '/ams';
$AMS_TRANS = $AMS_LIB . '/translations';
$AMS_CACHEDIR = $AMS_LIB . '/cache';
$AMS_PLUGINS = $AMS_LIB . '/plugins';
$AMS_TMPDIR = $AMS_LIB . '/tmp';
// Here your inc and func resides
$SITEBASE = $PUBLIC_PHP_PATH  . '/ams/' ;

//the paths to your website url
$BASE_WEBPATH = '/ams/';
$IMAGELOC_WEBPATH = $BASE_WEBPATH . 'img';
$WEBPATH = $BASE_WEBPATH . 'index.php';
$INGAME_WEBPATH = $BASE_WEBPATH . 'index.php';
$CONFIG_PATH = $PUBLIC_PHP_PATH;

//defines the default language
$DEFAULT_LANGUAGE = 'en';

//defines if logging actions should happen or not.
$TICKET_LOGGING = true;

//defines the time format display
$TIME_FORMAT = "m-d-Y H:i:s";

//defines which ingame layout template should be used
$INGAME_LAYOUT = "basic";

//forces to load the ingame templates if set to true
$FORCE_INGAME = false;

//file storage path (must be a publicly accessible url for 
$FILE_STORAGE_PATH = $PUBLIC_PHP_PATH . '/ams/files/';
$FILE_WEB_PATH = $BASE_WEBPATH . 'files/';

// Setup password
$NEL_SETUP_PASSWORD = '%nelSetupPassword%';

// Name of current domain (only if domain role)
$NEL_DOMAIN_NAME = '%nelDomainName%';
