-- FD Performance Monitoring SQL Dump
-- A. Kopmann, 3.12.2007
--
-- 
-- TODO:
--   - Add the other table definitions for the complete  
--     FD Performance Monitoring database
--
-- --------------------------------------------------------


-- 
-- Table orgaization for recording background data 
-- Create a database and execute the following sql commands
-- 


CREATE TABLE `BGRunTab` (
 `BGRunTabId` int(11) NOT NULL auto_increment,
 `DAQRunId` int(11) NOT NULL default '0',
 `EyeId` tinyint(11) unsigned NOT NULL default '0',
 `Filename` char(40) default '',
 `Type` enum('cal','run','test') default 'run',
 `TSample` smallint(5) unsigned NOT NULL default '0',
 `GPSStart` int(11) unsigned NOT NULL default '0',
 `GPSStop` int(11) unsigned NOT NULL default '0',
 `StatSamples` mediumint(5) unsigned NOT NULL default '0',
 `StatOffset` smallint(5) unsigned default NULL,
 `NSamples` smallint(5) unsigned default NULL,
 PRIMARY KEY (`BGRunTabId`)
) TYPE=MyISAM;


CREATE TABLE `BGSampleTab` (
 `BGSampleTabId` int(11) NOT NULL auto_increment,
 `BGRunTabId` int(11) unsigned NOT NULL default '0',
 `TelescopeId` tinyint(2) unsigned NOT NULL default '0',
 `Time` datetime, 
 `GPSSec` int(11) unsigned NOT NULL default '0',
 `GPSMiliSec` smallint(5) unsigned default NULL,
 `TReadout` smallint(5) unsigned default NULL,
 `TTransfer` smallint(5) unsigned default NULL,
 `TFile` smallint(5) unsigned default NULL,
 `TDatabase` smallint(5) unsigned default NULL,
 `MeanVariance` mediumint(5) unsigned default NULL,
 `VarVariance` mediumint(5) unsigned default NULL,
 `MeanThreshold` smallint(5) unsigned default NULL,
 `VarThreshold` smallint(5) unsigned default NULL,
 `MeanPedestal` mediumint(5) unsigned default NULL,
 `VarPedestal` mediumint(5) unsigned default NULL,
 `MeanHitrate` smallint(5) unsigned default NULL,
 `VarHitrate` smallint(5) unsigned default NULL,
 `NPixel` smallint(3) unsigned default NULL,
 `MeanVarianceV` mediumint(5) unsigned default NULL,
 `VarVarianceV` mediumint(5) unsigned default NULL,
 `MeanPedestalV` mediumint(5) unsigned default NULL,
 `VarPedestalV` mediumint(5) unsigned default NULL,
 `NPixelV` smallint(3) unsigned default NULL,
 PRIMARY KEY (`BGSampleTabId`)
) TYPE=MyISAM;


CREATE TABLE `BGPixelTab` (
 `BGSampleTabId` int(11) NOT NULL default '0',
 `PixelId` smallint(3) unsigned NOT NULL default '0',
 `Variance` mediumint(11) default NULL,
 `Threshold` smallint(5) unsigned default NULL,
 `Pedestal` mediumint(11) default NULL,
 `Hitrate` smallint(5) unsigned default NULL,
 PRIMARY KEY (`BGSampleTabId`, `PixelId`)
) TYPE=MyISAM;


CREATE TABLE `ErrorTab` (
 `ErrorTabId` int(11) NOT NULL auto_increment,
 `Time` datetime,
 `GPSSec` int(11) unsigned NOT NULL default '0',
 `EyeId` tinyint(2) unsigned NOT NULL default '0',
 `TelescopeId` tinyint(2) unsigned NOT NULL default '0',
 `SevLevel` enum('info','warn','error','critical','severe','fatal'),
 `Message` text,
 `Process` enum('Esm','Evb','EvbControl','processd','eyerc','feapid','readout','recorder','sender','unknown') ,
 `ErrorSourceTabId` int(11) unsigned,  
 PRIMARY KEY (`ErrorTabId`)
) TYPE=MyISAM;

