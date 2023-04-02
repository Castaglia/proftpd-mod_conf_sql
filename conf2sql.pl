#!/usr/bin/env perl

use strict;

use DBI;
use File::Basename qw(basename);
use Getopt::Long;

my $program = basename($0);
my $opts = {
  # Default table parameters
  'ctx-tab' => 'ftpctx',
  'conf-tab' => 'ftpconf',
  'map-tab' => 'ftpmap',

  # Default context name prefix to use
  'ctx-prefix' => 'ctx',
};

GetOptions($opts, 'add-conf', 'conf-tab=s', 'ctx-prefix=s', 'ctx-tab=s',
  'dbdriver=s', 'dbname=s', 'dbpass=s', 'dbserver=s', 'dbuser=s', 'dry-run',
  'help', 'map-tab=s', 'show-sql', 'verbose') or usage();

usage() if $opts->{help};

die "$program: missing --dbdriver option\n" unless defined($opts->{dbdriver});
if ($opts->{dbdriver} =~ /sqlite/i) {
  $opts->{dbdriver} = 'SQLite';

} elsif ($opts->{dbdriver} =~ /mysql/i) {
  $opts->{dbdriver} = 'mysql';

} elsif ($opts->{dbdriver} =~ /postgres/i) {
  $opts->{dbdriver} = 'Pg';
}

die "$program: missing --dbname option\n" unless defined($opts->{dbname});

# We need a database handle.
my ($dbname, $dbkey, $dsn);

# SQLite, unlike other databases, does not require server/user/pass options.
if ($opts->{dbdriver} =~ /sqlite/i) {
  $dbname = "$opts->{dbname}";
  $dbkey = 'dbname';
  $dsn = "DBI:$opts->{dbdriver}:$dbkey=$opts->{dbname}";

} else {
  die "$program: missing --dbpass option\n" unless defined($opts->{dbpass});
  die "$program: missing --dbserver option\n" unless defined($opts->{dbserver});
  die "$program: missing --dbuser option\n" unless defined($opts->{dbuser});

  # MySQL driver prefers 'database', Postgres likes 'dbname'
  $dbkey = ($opts->{dbdriver} =~ /mysql/i) ? 'database' : 'dbname';

  $dbname = "$opts->{dbname}\@$opts->{dbserver}";
  $dsn = "DBI:$opts->{dbdriver}:$dbkey=$opts->{dbname};host=$opts->{dbserver}";
}

my $dbi_opts = {
  RaiseError => 1,
  AutoCommit => 1,
};

my $dbh;
unless ($dbh = DBI->connect($dsn, $opts->{dbuser}, $opts->{dbpass},
    $dbi_opts)) {
  die "$program: unable to connect to $dbname: $DBI::errstr\n";
}

my $file = $ARGV[0];
open(my $conf, "< $file") or die "$program: unable to read $file: $!\n";

# The default/"server config" context
my $ctx = {
  name => $opts->{'ctx-prefix'} . '1',
  type => 'default',
  value => undef,
  directives => [],
  contexts => [],
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
    chomp($line);
    $line =~ /^<(\S+)(\s+)?(.*)?>.*$/;

    my ($type, $value) = ($1, $3);
    $ctxno++;
    my $sub_ctx = {
      name => $opts->{'ctx-prefix'} . $ctxno,
      type => $type,
      value => $value,
      directives => [],
      contexts => [],
    };

    push(@{ $ctx->{contexts} }, $sub_ctx);
    unshift(@ctxs, $sub_ctx);
    $ctx = $sub_ctx;

  # If the line starts with a '</', it's the end of the current context
  # (unless the current context is the default one, in which case it's
  # a syntax error in the config file.
  } elsif ($line =~ /^<\//) {
    chomp($line);
    $line =~ /^<\/(\S+).*?>$/;

    # Note: if the closing context name value doesn't match the current context
    # type value, it's a syntax error in the config
    if ($1 ne $ctx->{type}) {
      die "$program: syntax error in $file, line $i ($line): improperly nested contexts\n";
    }

    shift(@ctxs) if (scalar(@ctxs) > 1);
    $ctx = $ctxs[0];

  # Otherwise, it's a directive for the current context
  } else {
    chomp($line);

    if ($line =~ /^(\S+)\s+(.*)$/) {
      push(@{ $ctx->{directives} }, { name => $1, value => $2 });

    } else {
      push(@{ $ctx->{directives} }, { name => $line, value => '' });
    }
  }
}

close($conf);

# Now, generate the SQL statements necessary to import the given configuration
# file into a SQL database.

my ($stmt, $sth);

unless ($opts->{'add-conf'}) {
  $stmt = "DELETE FROM $opts->{'ctx-tab'}";
  $sth = dbi_prep_stmt($stmt);
  dbi_exec_stmt($sth);
  dbi_free_stmt($sth);

  $stmt = "DELETE FROM $opts->{'conf-tab'}";
  $sth = dbi_prep_stmt($stmt);
  dbi_exec_stmt($sth);
  dbi_free_stmt($sth);

  $stmt = "DELETE FROM $opts->{'map-tab'}";
  $sth = dbi_prep_stmt($stmt);
  dbi_exec_stmt($sth);
  dbi_free_stmt($sth);
}

$stmt = "INSERT INTO $opts->{'ctx-tab'} (parent_id, name, type, value) VALUES (NULL, ?, ?, NULL)";
$sth = dbi_prep_stmt($stmt);
$sth->bind_param(1, $opts->{'ctx-prefix'} . '1');
$sth->bind_param(2, 'default');
dbi_exec_stmt($sth);
dbi_free_stmt($sth);

$stmt = "SELECT id FROM $opts->{'ctx-tab'} WHERE type = ? AND value IS NULL";
$sth = dbi_prep_stmt($stmt);
$sth->bind_param(1, 'default');
dbi_exec_stmt($sth);
my $parent_id = ($sth->fetchrow_array())[0];
dbi_free_stmt($sth);

process_ctx($ctx, $parent_id);

exit 0;

# ---------------------------------------------------------------------------
sub dbi_prep_stmt {
  my ($stmt) = @_;
  my $sth;

  unless ($sth = $dbh->prepare($stmt)) {
    warn "$program: unable to prepare '$stmt': $DBI::errstr\n"
      if $opts->{verbose};
    return;
  }

  return $sth;
}

# ---------------------------------------------------------------------------
sub dbi_exec_stmt {
  my ($sth) = @_;

  if ($opts->{'show-sql'}) {
    my $params = $sth->{ParamValues};
    if ($params) {
      print "$program: executing: $sth->{Statement} using parameters $params\n";

    } else {
      print "$program: executing: $sth->{Statement}\n";
    }
  }

  unless ($opts->{'dry-run'}) {
    unless ($sth->execute()) {
      warn "$program: error executing '$sth->{Statement}': $DBI::errstr\n"
        if $opts->{verbose};
      return;
    }
  }

  return 1;
}

# ---------------------------------------------------------------------------
sub dbi_free_stmt {
  my ($sth) = @_;
  $sth->finish();
}

# ---------------------------------------------------------------------------
sub process_ctx {
  my ($ctx, $ctx_id) = @_;
  my ($stmt, $sth);

  # First, handle all of the configuration directives in this context
  foreach my $conf (@{ $ctx->{directives} }) {
    # Note: quoting is automatically handled by the placeholders
    my $name = $conf->{name};
    my $value = $conf->{value};

    $stmt = "INSERT INTO $opts->{'conf-tab'} (name, value) VALUES (?, ?)";
    $sth = dbi_prep_stmt($stmt);
    $sth->bind_param(1, $name);
    $sth->bind_param(2, $value);
    dbi_exec_stmt($sth);
    dbi_free_stmt($sth);

    $stmt = "SELECT id FROM $opts->{'conf-tab'} WHERE name = ? AND value = ?";
    $sth = dbi_prep_stmt($stmt);
    $sth->bind_param(1, $name);
    $sth->bind_param(2, $value);
    dbi_exec_stmt($sth);
    my $conf_id = ($sth->fetchrow_array())[0];
    dbi_free_stmt($sth);

    $stmt = "INSERT INTO $opts->{'map-tab'} (ctx_id, conf_id) VALUES (?, ?)";
    $sth = dbi_prep_stmt($stmt);
    $sth->bind_param(1, $ctx_id);
    $sth->bind_param(2, $conf_id);
    dbi_exec_stmt($sth);
    dbi_free_stmt($sth);
  }

  # Next, handle any contained contexts in this context
  foreach my $sub_ctx (@{ $ctx->{contexts} }) {
    my $name = $sub_ctx->{name};
    my $type = $sub_ctx->{type};
    my $value = $sub_ctx->{value};

    $stmt = "INSERT INTO $opts->{'ctx-tab'} (parent_id, name, type, value) VALUES (?, ?, ?, ?)";
    $sth = dbi_prep_stmt($stmt);
    $sth->bind_param(1, $ctx_id);
    $sth->bind_param(2, $name);
    $sth->bind_param(3, $type);
    $sth->bind_param(4, $value);
    dbi_exec_stmt($sth);
    dbi_free_stmt($sth);

    $stmt = "SELECT id FROM $opts->{'ctx-tab'} WHERE type = ? AND value = ? AND parent_id = ?";
    $sth = dbi_prep_stmt($stmt);
    $sth->bind_param(1, $type);
    $sth->bind_param(2, $value);
    $sth->bind_param(3, $ctx_id);
    dbi_exec_stmt($sth);
    my $sub_ctx_id = ($sth->fetchrow_array())[0];
    dbi_free_stmt($sth);

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

  --conf-tab              Default: $opts->{'conf-tab'}
  --ctx-tab               Default: $opts->{'ctx-tab'}
  --map-tab               Default: $opts->{'map-tab'}

 General Options:

  --add-conf              By default, $0 deletes all existing configuration
                          information in the tables before importing the
                          new information from the file.  Use this option
                          to retain existing information when importing
                          additional configurations.

  --ctx-prefix            Default: $opts->{'ctx-prefix'}

  --dry-run

  --help

  --verbose

END_OF_USAGE

  exit 0;
}
