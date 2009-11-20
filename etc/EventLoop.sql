# phpMyAdmin SQL Dump
# version 2.5.3
# http://www.phpmyadmin.net
#
# Host: localhost
# Erstellungszeit: 15. Januar 2007 um 16:41
# Server Version: 5.0.19
# PHP-Version: 4.3.3
# 
# Datenbank: `EventLoop`
# 

# --------------------------------------------------------

#
# Tabellenstruktur für Tabelle `adc`
#

CREATE TABLE `adc` (
  `AdcId` int(10) unsigned NOT NULL auto_increment,
  `RunId` int(20) default NULL,
  `EventId` int(11) NOT NULL,
  `Data` blob,
  PRIMARY KEY  (`AdcId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Adc traces';

# --------------------------------------------------------

#
# Tabellenstruktur für Tabelle `config`
#

CREATE TABLE `config` (
  `ConfigId` int(10) unsigned NOT NULL auto_increment,
  `RunId` int(10) unsigned NOT NULL,
  `Mode` varchar(8) NOT NULL,
  `Threshold` int(11) NOT NULL,
  `Gain` int(11) NOT NULL,
  PRIMARY KEY  (`ConfigId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Hardware configuration at run start';

# --------------------------------------------------------

#
# Tabellenstruktur für Tabelle `event`
#

CREATE TABLE `event` (
  `EventId` int(10) unsigned NOT NULL auto_increment,
  `RunId` int(10) unsigned NOT NULL,
  `Col` tinyint(3) unsigned NOT NULL,
  `Row` tinyint(3) unsigned NOT NULL,
  `Sec` int(10) unsigned NOT NULL,
  `SubSec` int(10) unsigned NOT NULL,
  `HardwareId` smallint(5) unsigned NOT NULL,
  `ChannelMap` int(10) unsigned NOT NULL,
  `Energy` int(11) NOT NULL,
  PRIMARY KEY  (`EventId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Event data';

# --------------------------------------------------------

#
# Tabellenstruktur für Tabelle `run`
#

CREATE TABLE `run` (
  `RunId` int(8) unsigned NOT NULL auto_increment,
  `Location` tinyint(8) NOT NULL,
  `Type` varchar(8) NOT NULL,
  `StartTime` int(10) unsigned default NULL,
  `StopTime` int(8) unsigned NOT NULL,
  `Status` varchar(8) NOT NULL,
  `Comment` varchar(256) NOT NULL,
  PRIMARY KEY  (`RunId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Run table for testing eventloop ';
