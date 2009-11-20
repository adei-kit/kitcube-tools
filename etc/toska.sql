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
  `ExperimentId` int(10) unsigned NOT NULL  auto_increment,
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
  `RunId` int(10) unsigned NOT NULL  auto_increment,
  `ModuleId` int(10) unsigned NOT NULL default '0',
  `Name` int(10) unsigned NOT NULL  default '0',
  `Description` varchar(256) collate latin1_general_ci NOT NULL default '',
  `StartTime` datetime, 
  `StopTime` datetime, 
  `Level` tiny(3) unsigned NOT NULL default '0',
  `Config` longtext,
  PRIMARY KEY  (`RunId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Table `Sensor`
-- 

CREATE TABLE `Sensor` (
  `SensorId` int(10) unsigned NOT NULL  auto_increment,
  `Name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `Description` varchar(256) collate latin1_general_ci NOT NULL default '',
  `Unit` varchar(50) collate latin1_general_ci NOT NULL default '',
  PRIMARY KEY  (`SensorId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- Table `Module`
--

CREATE TABLE `Module` (
  `ModuleId` int(10) unsigned NOT NULL  auto_increment,
  `Name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `Description` varchar(256) collate latin1_general_ci NOT NULL default '',
  `Type` enum('Sampled', 'Triggered') default 'Sampled',
  `TSample` double default '1',  
  `Config` longtext,
   PRIMARY KEY (`ModuleId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;


-- --------------------------------------------------------

-- 
-- Table `Link`
-- 

CREATE TABLE `Link` (
  `LinkId` int(10) unsigned NOT NULL  auto_increment,
  `ModuleId` int(10) unsigned NOT NULL default '0',
  `ChannelId` int(10) unsigned NOT NULL default '0',
  `SensorId` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY (`LinkId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;


-- --------------------------------------------------------

-- 
-- Table `View`
-- 

CREATE TABLE `View` (
  `ViewId` int(10) unsigned NOT NULL auto_increment,
  `ModuleId` int(10) unsigned NOT NULL default '0',
  `Name` varchar(50) collate latin1_general_ci NOT NULL default '',
  `Description` varchar(50) collate latin1_general_ci NOT NULL default '',
  PRIMARY KEY (`ViewId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;




-- --------------------------------------------------------

-- 
-- Table `Data001`
-- 

CREATE TABLE `Data001` (
  `DataId` int(10) unsigned NOT NULL  auto_increment,
  `Time` datetime,
  `NanoSec` int unsigned default '0',
  `col000` double default '0',
  `col001` double default '0',
  `col002` double default '0',
  `col003` double default '0',
  PRIMARY KEY (`DataId`) 
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

-- --------------------------------------------------------

-- 
-- View for data table of module with id 001 'View001'
-- 

CREATE VIEW `View001` AS SELECT 
   `DataId`,
   `Time`,
   `NanoSec`,
   `col000` AS `W4310`,
   `col001` AS `W4320`,
   `col002` AS `W4330`,
   `col003` AS `W4340` FROM `Data001`;


-- --------------------------------------------------------

-- 
-- View for data table of module with id 001 'View002'
-- 

CREATE VIEW `View002` AS SELECT 
   `DataId`,
   `Time`,
   `NanoSec`,
   `col000` AS `W4310`,
   `col001` AS `W4320`,
   `col002` AS `W4330`,
   `col003` AS `W4340` FROM `Data001` WHERE 'Time' = (select max(Time) from Data001);


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


