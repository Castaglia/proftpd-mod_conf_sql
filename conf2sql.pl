#!/usr/bin/env perl

use strict;

use DBI;
use File::Basename qw(basename);
use Getopt::Long;

my $program = basename($0);
my %opts = (
  # Default table parameters
  'ctx-tab' => 'ftpctx',
  'conf-tab' => 'ftpconf',
  'map-tab' => 'ftpmap',

  # Default context name prefix to use
  'ctx-prefix' => 'ctx',
);

GetOptions(\%opts, 'add-conf', 'conf-tab=s', 'ctx-prefix=s', 'ctx-tab=s',
  'dbdriver=s', 'dbname=s', 'dbpass=s', 'dbserver=s', 'dbuser=s', 'dry-run',
  'help', 'map-tab=s', 'show-sql', 'verbose') or usage();

usage() if $opts{'help'};

die "$program: missing --dbdriver option\n" unless defined($opts{'dbdriver'});
die "$program: missing --dbname option\n" unless defined($opts{'dbname'});
die "$program: missing --dbpass option\n" unless defined($opts{'dbpass'});
die "$program: missing --dbserver option\n" unless defined($opts{'dbserver'});
die "$program: missing --dbuser option\n" unless defined($opts{'dbuser'});

# We need a database handle.
my $dbname = "$opts{'dbname'}\@$opts{'dbserver'}";

# MySQL driver prefers 'database', Postgres likes 'dbname'
my $dbkey = ($opts{'dbdriver'} =~ /mysql/i) ? 'database' : 'dbname';
my $dsn = "DBI:$opts{'dbdriver'}:$dbkey=$opts{'dbname'};host=$opts{'dbserver'}";

my $dbh;
unless ($dbh = DBI->connect($dsn, $opts{'dbuser'}, $opts{'dbpass'})) {
  die "$program: unable to connect to $dbname: $DBI::errstr\n";
}

my $file = $ARGV[0];
open(my $conf, "< $file") or die "$program: unable to read $file: $!\n";

# The default/"server config" context
my $ctx = {
  'name' => $opts{'ctx-prefix'} . '1',
  'key' => 'default',
  'value' => undef,
  'directives' => [],
  'contexts' => [],
};

my @ctxs = ();
unshift(@ctxs, $ctx);

my $i = 0;
my $ctxno = 1;
while (chomp(my $line = <$conf>)) {
  $i++;
 
  # Skip comments and blank lines
  next if $line =~ /^(\s{0,})?#/;
  next if $line =~ /^\s{0,}$/;

  # Trim leading, trailing whitespace
  $line =~ s/^\s{0,}//;
  $line =~ s/\s{0,}$//;
 
  # Check for line continuations, and concatenate the next line.
  while ($line =~ /\\$/) {

    chomp(my $nextline = <$conf>);
    $i++;

    # Trim leading, trailing whitespace
    $nextline =~ s/^\s{0,}//;
    $nextline =~ s/\s{0,}$//;

    # Remove the line continuation mark
    $line =~ s/\\$//;

    $line .= $nextline;
  }

  # If the line starts with a '<' (and not a '</'), it's the start of a new
  # context.
  if ($line =~ /^<[^\/]/) {
    $line =~ /^<(\S+)(\s+)?(.*)?>.*$/;

    my ($key, $value) = ($1, $3);
    $ctxno++;
    my $sub_ctx = {
      'name' => $opts{'ctx-prefix'} . $ctxno,
      'key' => $key,
      'value' => $value,
      'directives' => [],
      'contexts' => [],
    };

    push(@{ $ctx->{'contexts'} }, $sub_ctx);
    unshift(@ctxs, $sub_ctx);
    $ctx = $sub_ctx;

  # If the line starts with a '</', it's the end of the current context
  # (unless the current context is the default one, in which case it's
  # a syntax error in the config file.
  } elsif ($line =~ /^<\//) {
    $line =~ /^<\/(\S+).*?>$/;

    # Note: if the closing context name value doesn't match the current context
    # key value, it's a syntax error in the config
    if ($1 ne $ctx->{'key'}) {
      die "$program: syntax error in $file, line $i ($line): improperly nested contexts\n";
    }

    shift(@ctxs) if (scalar(@ctxs) > 1);
    $ctx = $ctxs[0];

  # Otherwise, it's a directive for the current context
  } else {

    if ($line =~ /^(\S+)\s+(.*)$/) {
      push(@{ $ctx->{'directives'} }, { 'key' => $1, 'value' => $2 });

    } else {
      push(@{ $ctx->{'directives'} }, { 'key' => $line, 'value' => '' }); 
    }
  }
}

close($conf);

# Now, generate the SQL statements necessary to import the given configuration
# file into a SQL database.

my ($sql, $sth);

unless ($opts{'add-conf'}) {
  $sql = "DELETE FROM $opts{'ctx-tab'}";
  $sth = dbi_prep_sql($sql);
  dbi_exec_sql($sql, $sth);
  dbi_free_sql($sth);

  $sql = "DELETE FROM $opts{'conf-tab'}";
  $sth = dbi_prep_sql($sql);
  dbi_exec_sql($sql, $sth);
  dbi_free_sql($sth);

  $sql = "DELETE FROM $opts{'map-tab'}";
  $sth = dbi_prep_sql($sql);
  dbi_exec_sql($sql, $sth);
  dbi_free_sql($sth);
}

$sql = "INSERT INTO $opts{'ctx-tab'} (parent_id, name, type, info) VALUES (NULL, '" . ($opts{'ctx-prefix'} . '1') . "', 'default', NULL)";
$sth = dbi_prep_sql($sql);
dbi_exec_sql($sql, $sth);
dbi_free_sql($sth);

$sql = "SELECT id FROM $opts{'ctx-tab'} WHERE type = 'default' AND info IS NULL";
$sth = dbi_prep_sql($sql);
dbi_exec_sql($sql, $sth);
my $parent_id = ($sth->fetchrow_array())[0];
dbi_free_sql($sth);

process_ctx($ctx, $parent_id);

exit 0;

# ---------------------------------------------------------------------------
sub dbi_prep_sql {
  my ($sql) = @_;
  my $sth;

  unless ($sth = $dbh->prepare($sql)) {
    warn "$program: unable to prepare '$sql': $DBI::errstr\n"
      if $opts{'verbose'};
    return;
  }

  return $sth;
}

# ---------------------------------------------------------------------------
sub dbi_exec_sql {
  my ($sql, $sth) = @_;

  print "$program: executing: $sql\n" if $opts{'show-sql'};

  unless ($opts{'dry-run'}) {
    unless ($sth->execute()) {
      warn "$program: error executing '$sql': $DBI::errstr\n"
        if $opts{'verbose'};
      return;
    }
  }

  return 1;
}

# ---------------------------------------------------------------------------
sub dbi_free_sql {
  my ($sth) = @_;
  $sth->finish();
}

# ---------------------------------------------------------------------------
sub process_ctx {
  my ($ctx, $ctx_id) = @_;
  my ($sql, $sth);

  # First, handle all of the configuration directives in this context
  foreach my $conf (@{ $ctx->{'directives'} }) {
    my $key = $dbh->quote($conf->{'key'});
    my $value = $dbh->quote($conf->{'value'});

    $sql = "INSERT INTO $opts{'conf-tab'} (type, info) VALUES ($key, $value)";
    $sth = dbi_prep_sql($sql);
    dbi_exec_sql($sql, $sth);
    dbi_free_sql($sth);

    $sql = "SELECT id FROM $opts{'conf-tab'} WHERE type = $key AND info = $value";
    $sth = dbi_prep_sql($sql);
    dbi_exec_sql($sql, $sth);
    my $conf_id = ($sth->fetchrow_array())[0];
    dbi_free_sql($sth);

    $sql = "INSERT INTO $opts{'map-tab'} (ctx_id, conf_id) VALUES ($ctx_id, $conf_id)";
    $sth = dbi_prep_sql($sql);
    dbi_exec_sql($sql, $sth);
    dbi_free_sql($sth);
  }

  # Next, handle any contained contexts in this context
  foreach my $sub_ctx (@{ $ctx->{'contexts'} }) {
    my $name = $dbh->quote($sub_ctx->{'name'});
    my $key = $dbh->quote($sub_ctx->{'key'});
    my $value = $dbh->quote($sub_ctx->{'value'});

    $sql = "INSERT INTO $opts{'ctx-tab'} (parent_id, name, type, info) VALUES ($ctx_id, $name, $key, $value)";
    $sth = dbi_prep_sql($sql);
    dbi_exec_sql($sql, $sth);
    dbi_free_sql($sth);

    $sql = "SELECT id FROM $opts{'ctx-tab'} WHERE type = $key AND info = $value AND parent_id = $ctx_id";
    $sth = dbi_prep_sql($sql);
    dbi_exec_sql($sql, $sth);
    my $sub_ctx_id = ($sth->fetchrow_array())[0];
    dbi_free_sql($sth);

    process_ctx($sub_ctx, $sub_ctx_id);
  }
}

# ---------------------------------------------------------------------------
sub usage {

  print STDOUT <<END_OF_USAGE;

usage: $program [options] config-file

 Database Options:

  --dbdriver              DBD driver name , e.g. 'mysql'.  Required.
  --dbname                Database name.  Required.
  --dbpass                Database user password.  Required.
  --dbserver              Database server.  Required.
  --dbuser                Database user.  Required.

 Table Options:

  --conf-tab              Default: $opts{'conf-tab'}
  --ctx-tab               Default: $opts{'ctx-tab'}
  --map-tab               Default: $opts{'map-tab'}

 General Options:

  --add-conf              By default, $0 deletes all existing configuration
                          information in the tables before importing the
                          new information from the file.  Use this option
                          to retain existing information when importing
                          additional configurations.

  --ctx-prefix            Default: $opts{'ctx-prefix'}

  --dry-run

  --help

  --verbose

END_OF_USAGE

  exit 0;
}
