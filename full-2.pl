# handle differences between MySQL & SQLITE's string escaping
sub escape {
  my $string = shift;
  $string =~ s/\\\\'/'/g; 
  $string =~ s/\\'/''/g; 
  $string =~ s/\\"/"/g; 
  $string =~ s/\\(r|n)/ /g;
  return $string;
}

# count lines (for better output messages)
$lines = 0;
open FILE, "<full-2.sql" or die;
while (sysread FILE, $buffer, 4096) { $lines += ($buffer =~ tr/\n//); }
close FILE;

# parse the GCD data-dump (schema 2 format)
print STDERR "Parsing input data...\n";
open FILE, "<full-2.sql" or die;
LINE: while(<FILE>) {
  $line++;
  
  # convert CREATE TABLE statements to SQLITE syntax
  if(substr($_,0,12) eq "CREATE TABLE") {
    if(not(/(core_indexer|core_reservation|core_seriescredit|core_sequence)/)) {
      $state = 1; 
      s/\n//;
      $create = $_;
      $columns = "";
    }
  } elsif($state == 1) {
    if(substr($_,0,15) eq ") ENGINE=MyISAM") {
      $state = 0; 
      print "$create$columns);\n";
    } else {
      s/ int\(11\) NOT NULL auto_increment,/ INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,/;
      s/ COMMENT '.*$//;
      s/,[ \t]*$//;
      s/\n//;
      if(not(/^[ ]*(PRIMARY)? KEY/)) {
        $columns .= "," if($columns ne "");
        $columns .= $_;
      }
    }
  }
  
  # INSERT statements need to be broken up; SQLITE only handles a 
  # single row inserted per statement.  
  elsif(substr($_,0,11) eq "INSERT INTO") {
    my @insert = split/\),\(/;                                                # split the values
    my $firstitem = shift @insert;                                            # pull out the first item
    my $lastitem = pop @insert;                                               # pull out the last item
    my $db = $1 if($firstitem =~ /INSERT INTO `([^`]+)`/);                    # get the database name
    next LINE if($db =~ /(core_indexer|core_reservation|core_seriescredit|core_sequence)/); # skip these tables

    printf STDERR "%0.2f%%, INSERTING INTO $db                              \r", $line/$lines*100;
    print "BEGIN TRANSACTION;\n";
    print escape($firstitem),");\n";
    foreach(@insert) {
      print "INSERT INTO `$db` VALUES (",escape($_),");\n";
    }
    print "INSERT INTO `$db` VALUES (",escape($lastitem);
    print "END TRANSACTION;\n\n\n";
  }
}
close FILE;
print STDERR "100.00%                                            \n";

# We've now imported all the raw data; time to convert it to 
# the format we want:
print STDERR "Creating output tables...\n";

print << 'EOF';
CREATE TABLE `covers` (
  `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  `issue_id` int(11) NOT NULL,
  `has_image` tinyint(1) NOT NULL default '0',
  `marked` tinyint(1) NOT NULL default '0',
  `has_small` tinyint(1) NOT NULL default '0',
  `has_medium` tinyint(1) NOT NULL default '0',
  `has_large` tinyint(1) NOT NULL default '0'
);
INSERT INTO `covers` 
SELECT `id`, `issue_id`, `has_image`, `marked`, `has_small`, `has_medium`, `has_large` 
FROM core_cover;
DROP TABLE `core_cover`;

CREATE TABLE `issues` (
  `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  `number` varchar(50) NOT NULL,
  `series_id` int(11) NOT NULL,
  `sort_code` varchar(50) NOT NULL default '000',
  `publication_date` varchar(255) default NULL,
  `price` varchar(255) default NULL,
  `page_count` varchar(10) default NULL,
  `notes` mediumtext,
  `sequence_count` int(11) NOT NULL default '0'
);
INSERT INTO `issues` 
SELECT `id`, `number`, `series_id`, `sort_code`, `publication_date`, `price`, `page_count`, `notes`, `sequence_count` 
FROM core_issue;
DROP TABLE `core_issue`;

CREATE TABLE `series` (
  `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  `name` varchar(255) NOT NULL,
  `format` varchar(255) default NULL,
  `year_began` int(11) NOT NULL,
  `year_ended` int(11) default NULL,
  `publication_dates` varchar(255) default NULL,
  `publisher_id` int(11) NOT NULL,
  `imprint_id` int(11) default NULL,
  `first_issue` varchar(25) default NULL,
  `last_issue` varchar(25) default NULL,
  `tracking_notes` mediumtext,
  `notes` mediumtext
);
INSERT INTO `series` 
SELECT `id`, `name`, `format`, `year_began`, `year_ended`, `publication_dates`, `publisher_id`, `imprint_id`, `first_issue`, `last_issue`, `tracking_notes`, `notes` 
FROM `core_series`;
DROP TABLE `core_series`;

CREATE TABLE `publishers` (
  `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  `name` varchar(255) NOT NULL,
  `country_id` int(11) default NULL,
  `year_began` int(11) default NULL,
  `year_ended` int(11) default NULL,
  `notes` longtext,
  `url` varchar(255) default NULL,
  `parent_id` int(11) default NULL
);
INSERT INTO `publishers` SELECT `id`, `name`, `country_id`, `year_began`, `year_ended`, `notes`, `url`, `parent_id` FROM `core_publisher`;
DROP TABLE `core_publisher`;

CREATE INDEX "issue_series" on issues (series_id ASC);
EOF

print STDERR "Vacuuming...\n";
print << 'EOF';
PRAGMA page_size = 16384; VACUUM;
EOF

