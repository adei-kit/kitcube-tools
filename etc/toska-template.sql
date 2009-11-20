-- phpMyAdmin SQL Dump
-- version 2.7.0-pl1
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Erstellungszeit: 21. Juni 2006 um 14:52
-- Server Version: 5.0.18
-- PHP-Version: 4.4.1-pl1
-- 
-- Datenbank: `toska`
-- 

-- --------------------------------------------------------

-- 
-- Table `Config`
-- 

CREATE TABLE `Experiment` (
  `ExperimentId` int(10) unsigned NOT NULL default '0',
  `Description` varchar(50) collate latin1_general_ci NOT NULL default '',
  `StartTime` datetime, 
  `StopTime` datetime, 
  `Config` longtext,                                   
  PRIMARY KEY  (`ExperimentId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Table `Run`
-- 

CREATE TABLE `Run` (
  `RunId` int(10) unsigned NOT NULL default '0',
  `Description` varchar(50) collate latin1_general_ci NOT NULL default '',
  `StartTime` datetime, 
  `StopTime` datetime, 
  `Type` enum('Run', 'Sequence') default 'Run',
  `Config` longtext,
  PRIMARY KEY  (`RunId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Table `Sensor`
-- 

CREATE TABLE `Sensor` (
  `SensorId` int(10) unsigned NOT NULL default '0',
  `Name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `Description` varchar(50) collate latin1_general_ci NOT NULL default '',
  `Unit` varchar(50) collate latin1_general_ci NOT NULL default '',
  PRIMARY KEY  (`SensorId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Table `Module`
--

CREATE TABLE `Link` (
  `ModuleId` int(10) unsigned NOT NULL default '0',
  `Name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `Description` varchar(50) collate latin1_general_ci NOT NULL default '',
  `Type` enum('Sampled', 'Triggered') default 'Sampled',
  `TSample` double default '1',  
  KEY `LinkId`
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;


-- --------------------------------------------------------

-- 
-- Table `Link`
-- 

CREATE TABLE `Link` (
  `LinkId` int(10) unsigned NOT NULL default '0',
  `ModuleId` int(10) unsigned NOT NULL default '0',
  `ChannelId` int(10) unsigned NOT NULL default '0',
  `SensorId` int(10) unsigned NOT NULL default '0',
  KEY `LinkId`
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;


-- --------------------------------------------------------

-- 
-- Table `View`
-- 

CREATE TABLE `View` (
  `ViewId` int(10) unsigned NOT NULL default '0',
  `ModuleId` int(10) unsigned NOT NULL default '0',
  `Name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `Description` varchar(50) collate latin1_general_ci NOT NULL default '',
  KEY `ViewId`
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;




-- --------------------------------------------------------

-- 
-- Table `Data001`
-- 

CREATE TABLE `Data001` (
  `DataId` int(10) unsigned NOT NULL default '0',
  `Time` datetime,
  `NanoSec` int unsigned default '0',
  `col000` double default '0',
  `col001` double default '0',
  `col002` double default '0',
  `col003` double default '0',
  KEY `DataId` 
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- View for data table of module with id 001 'View001'
-- 
CREATE VIEW v AS SELECT qty, price, qty*price AS value FROM t;

CREATE VIEW `View001` AS SELECT (
   `DataId`,
   `Time`,
   `NanoSec`,
   `col000`,
   `col001`,
   `col002`,
   `col003`
) AS (
   `DataId`,
   `Time`,
   `NanoSec`,
   `W4310`, 
   `W4320`,
   `W4330`,
   `W4340`,
) FROM `Data001`;


-- --------------------------------------------------------

-- 
-- Table `SysLog`
-- 

CREATE TABLE `SysLog` (
  `SysLogId` bigint(20) NOT NULL auto_increment,
  `Time` datetime,
  `Sender` varchar(250) collate latin1_general_ci NOT NULL,
  `Message` text collate latin1_general_ci NOT NULL,
  PRIMARY KEY  (`SysLogId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=10025 ;


-- --------------------------------------------------------

-- 
-- Tabellenstruktur für Tabelle `items`
-- 

CREATE TABLE `items` (
  `iid` int(10) unsigned NOT NULL default '0',
  `name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `formular` varchar(255) collate latin1_general_ci NOT NULL default '',
  `inputs` blob NOT NULL,
  PRIMARY KEY  (`iid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Tabellenstruktur für Tabelle `marker`
-- 

CREATE TABLE `marker` (
  `start` double NOT NULL default '0',
  `name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `stop` double NOT NULL default '0',
  `experiment` varchar(50) collate latin1_general_ci NOT NULL default '',
  `text` text collate latin1_general_ci NOT NULL,
  PRIMARY KEY  (`start`),
  KEY `experiment` (`experiment`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Tabellenstruktur für Tabelle `masks`
-- 

CREATE TABLE `masks` (
  `maskid` smallint(5) unsigned NOT NULL default '0',
  `name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `gid` smallint(5) unsigned NOT NULL default '0',
  `mask` blob NOT NULL,
  PRIMARY KEY  (`maskid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Tabellenstruktur für Tabelle `messagelog`
-- 

CREATE TABLE `messagelog` (
  `lid` bigint(20) NOT NULL auto_increment,
  `messageid` int(10) NOT NULL default '0',
  `name` text collate latin1_general_ci NOT NULL,
  `mtype` tinyint(4) NOT NULL,
  `cond` text collate latin1_general_ci NOT NULL,
  `ack` tinyint(4) NOT NULL,
  `come` double NOT NULL default '0',
  `go` double NOT NULL default '0',
  `acked` double NOT NULL,
  PRIMARY KEY  (`lid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=31 ;

-- --------------------------------------------------------

-- 
-- Tabellenstruktur für Tabelle `messages`
-- 

CREATE TABLE `messages` (
  `messageid` smallint(5) unsigned NOT NULL default '0',
  `name` text collate latin1_general_ci NOT NULL,
  `mtype` tinyint(3) unsigned NOT NULL default '0',
  `bid` int(10) NOT NULL default '0',
  `bindex` smallint(5) NOT NULL default '0',
  `trigger` tinyint(3) unsigned NOT NULL default '0',
  `threshold` double NOT NULL default '0',
  `hyst` double NOT NULL,
  `ack` tinyint(3) NOT NULL,
  `signal` tinyint(3) NOT NULL,
  PRIMARY KEY  (`messageid`),
  UNIQUE KEY `meldid` (`messageid`),
  KEY `meldid_2` (`messageid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Tabellenstruktur für Tabelle `opc`
-- 

CREATE TABLE `opc` (
  `oid` int(10) unsigned NOT NULL default '0',
  `name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `hardware` text collate latin1_general_ci NOT NULL,
  `length` smallint(5) unsigned NOT NULL default '0',
  `opcurl` text collate latin1_general_ci NOT NULL,
  `mode` tinyint(3) NOT NULL default '0',
  PRIMARY KEY  (`oid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Tabellenstruktur für Tabelle `syslog`
-- 

CREATE TABLE `syslog` (
  `lid` bigint(20) NOT NULL auto_increment,
  `ts` double NOT NULL default '0',
  `source` varchar(250) collate latin1_general_ci NOT NULL,
  `message` text collate latin1_general_ci NOT NULL,
  PRIMARY KEY  (`lid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=10025 ;
