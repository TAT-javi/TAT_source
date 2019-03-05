--
-- Host: localhost    Database: shift_fov
-- ------------------------------------------------------
--
-- Create database 'fov'
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `fov` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `fov`;

--
-- Table structure for table `Shift`
--

DROP TABLE IF EXISTS `Shift`;
CREATE TABLE `Shift` (
  `Id` MEDIUMINT NOT NULL AUTO_INCREMENT,
  `Timestamp` int(12) unsigned NOT NULL default '0',
  `Status` text(32) NOT NULL,
  `RA` int(6)  NOT NULL default '0',
  `Declination` int(6) NOT NULL default '0',
  `R` decimal(8,7) unsigned NOT NULL default '0.00000',
  `Frame` text(300) NOT NULL,
  PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Give permissions

grant all on fov.* to writer@localhost identified by 'secret';
grant select on fov.* to fov@localhost identified by 'public';



