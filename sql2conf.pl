#!/usr/bin/env perl
# ---------------------------------------------------------------------------
#  $Id: sql2conf.pl,v 1.3 2003/07/02 22:16:32 tj Exp tj $
# ---------------------------------------------------------------------------

use strict;

use DBI;
use File::Basename qw(basename);
use Getopt::Long;

my $program = basename($0);
my %opts = (

  # Default table parameters
  'ctxt-tab' => 'ftpctxt',
  'conf-tab' => 'ftpconf',
  'map-tab' => 'ftpmap',
);

GetOptions(\%opts, 'conf-tab=s', 'ctxt-tab=s', 'dbdriver=s', 'dbname=s',
  'dbpass=s', 'dbserver=s', 'dbuser=s', 'help', 'map-tab=s', 'show-sql',
  'show-tags', 'verbose');

usage() if $opts{'help'};

die "$program: missing --dbdriver option\n" unless defined($opts{'dbdriver'});
die "$program: missing --dbname option\n" unless defined($opts{'dbname'});
if (!$opts{'dbdriver'} =~ /sqlite/i) {
die "$program: missing --dbpass option\n" unless defined($opts{'dbpass'});
die "$program: missing --dbserver option\n" unless defined($opts{'dbserver'});
die "$program: missing --dbuser option\n" unless defined($opts{'dbuser'});

# We need a database handle.
my $dbname = "$opts{'dbname'}\@$opts{'dbserver'}";
}

# MySQL driver prefers 'database', Postgres and SQLite likes 'dbname'
my $dbkey = ($opts{'dbdriver'} =~ /mysql/i) ? 'database' : 'dbname';
my $dsn;
# if sqlite not use host
if ($opts{'dbdriver'} =~ /sqlite/i) {
$dsn = "DBI:$opts{'dbdriver'}:$dbkey=$opts{'dbname'}";
} else {
$dsn = "DBI:$opts{'dbdriver'}:$dbkey=$opts{'dbname'};host=$opts{'dbserver'}";
}

my $dbh;
unless ($dbh = DBI->connect($dsn, $opts{'dbuser'}, $opts{'dbpass'})) {
  die "$program: unable to connect to $dbname: $DBI::errstr\n";
}

# Given a context ID, find all directives that map to this ID, then
# find all the contexts for which this context is the parent.  For each
# of those contexts, find all directives, etc....

my $conf = get_ctxt();

show_conf($conf);

exit 0;

# ---------------------------------------------------------------------------
sub dbi_prep_sql {
  my ($sql) = @_;
  my $sth;

  unless ($sth = $dbh->prepare($sql)) {
    warn "$program: unable to prepare '$sql': $DBI::errstr\n";
    return;
  }

  return $sth;
}

# ---------------------------------------------------------------------------
sub dbi_exec_sql {
  my ($sql, $sth) = @_;

  print "$program: executing: $sql\n" if $opts{'show-sql'};

  unless ($sth->execute()) {
    warn "$program: error executing '$sql', $DBI::errstr\n";
    return;
  }

  return 1;
}

# ---------------------------------------------------------------------------
sub dbi_free_sql {
  my ($sth) = @_;
  $sth->finish();
}

# ---------------------------------------------------------------------------
sub get_ctxt {
  my ($id) = @_;
  my $ctxt;

  # If no context ID is given, look up the default/"server config" context.
  # It will be the one that has no parent (i.e. parent_id is NULL).

  my $ctxt_tab = $opts{'ctxt-tab'};

  unless ($id) {
    my $sql = "SELECT id FROM $ctxt_tab WHERE parent_id IS NULL";
    my $sth = dbi_prep_sql($sql);
    dbi_exec_sql($sql, $sth);
    $id = ($sth->fetchrow_array())[0];
    dbi_free_sql($sth);
  }

  my $sql = "SELECT type, info FROM $ctxt_tab WHERE id = $id";
  my $sth = dbi_prep_sql($sql);
  dbi_exec_sql($sql, $sth);

  $ctxt->{'id'} = $id;
  ($ctxt->{'key'}, $ctxt->{'value'}) = ($sth->fetchrow_array())[0, 1];
  dbi_free_sql($sth);

  $ctxt->{'directives'} = get_ctxt_directives($id);

  my $ctxts = get_ctxt_ctxts($id);

  foreach $id (@{ $ctxts }) {
    push(@{ $ctxt->{'contexts'} }, get_ctxt($id));
  }

  return $ctxt;
}

# ---------------------------------------------------------------------------
sub get_ctxt_ctxts {
  my ($ctxt_id) = @_;

  my $ctxt_tab = $opts{'ctxt-tab'};
  my $sql = "SELECT id FROM $ctxt_tab WHERE parent_id = $ctxt_id";

  my $sth = dbi_prep_sql($sql);
  dbi_exec_sql($sql, $sth);

  my $ctxts;
  while (my @row = $sth->fetchrow_array()) {
    my ($id) = @row;
    push(@{ $ctxts }, $id);
  }

  dbi_free_sql($sth);
  return $ctxts;
}

# ---------------------------------------------------------------------------
sub get_ctxt_directives {
  my ($ctxt_id) = @_;

  my $dir_tab = $opts{'conf-tab'};
  my $map_tab = $opts{'map-tab'};

  my $sql = "SELECT id, type, info FROM $dir_tab INNER JOIN $map_tab " .
            " ON $dir_tab.id = $map_tab.conf_id" .
            " WHERE $map_tab.ctxt_id = $ctxt_id";

  my $sth = dbi_prep_sql($sql);
  dbi_exec_sql($sql, $sth);

  my $directives;
  while (my @row = $sth->fetchrow_array()) {
    my ($id, $key, $value) = @row;
    push(@{ $directives }, { 'id' => $id, 'key' => $key, 'value' => $value });
  }

  dbi_free_sql($sth);
  return $directives;
}

# ---------------------------------------------------------------------------
sub show_conf {
  my ($ctxt) = @_;

  show_directives($ctxt->{'directives'}, 0);
  show_ctxts($ctxt->{'contexts'}, 0);
}

# ---------------------------------------------------------------------------
sub show_ctxts {
  my ($ctxts, $indent) = @_;

  foreach my $ctxt (@{ $ctxts }) {
    print STDOUT "\n";
    print STDOUT ' ' x $indent, "<$ctxt->{'key'}",
      $ctxt->{'value'} ? " $ctxt->{'value'}" : '', ">";

    if ($opts{'show-tags'}) {
      print STDOUT " ($opts{'ctxt-tab'}, id $ctxt->{'id'})";
    }

    print STDOUT "\n";

    show_directives($ctxt->{'directives'}, $indent + 2);
    show_ctxts($ctxt->{'contexts'}, $indent + 2);

    print STDOUT ' ' x $indent, "</$ctxt->{'key'}>\n";
  }
}

# ---------------------------------------------------------------------------
sub show_directives {
  my ($directives, $indent) = @_;

  foreach my $directive (@{ $directives }) {
    print STDOUT ' ' x $indent, "$directive->{'key'} $directive->{'value'}";

    if ($opts{'show-tags'}) {
      print STDOUT " ($opts{'conf-tab'}, id $directive->{'id'})";
    }

    print STDOUT "\n";
  }
}

# ---------------------------------------------------------------------------
sub usage {

  print STDOUT <<END_OF_USAGE;

usage: $program [options]

 Database Options:

  --dbdriver              DBD driver name , e.g. 'mysql'.  Required.
  			  Valid value;
			  - mysql ==> for mysql database
			  - Pg ==> for postgresql database
			  - SQLite ==> for sqlite database
  --dbname                Database name.  Required.
  			  if dbdriver are SQLite dbname a used to specify the sqlite database path

  			  The dbpass, dbserver, dbuser are not required for sqlite.
  --dbpass                Database user password.  Required.
  --dbserver              Database server.  Required.
  --dbuser                Database user.  Required.

 Table Options:

  --conf-tab                   Default: $opts{'conf-tab'}
  --ctxt-tab                   Default: $opts{'ctxt-tab'}
  --map-tab                    Default: $opts{'map-tab'}

 General Options:

  --help
  --show-sql
  --show-tags
  --verbose

END_OF_USAGE

  exit 0;
}
