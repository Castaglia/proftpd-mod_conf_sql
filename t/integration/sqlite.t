#!/usr/bin/env perl

use strict;

use Carp;
use Cwd qw(abs_path realpath);
use File::Path qw(mkpath rmtree);
use File::Spec;
use Test::Simple tests => 7;

my $tmpdir = $ARGV[0];
my $proftpd = $ENV{PROFTPD_TEST_BIN};

my $test_dir = (File::Spec->splitpath(abs_path(__FILE__)))[1];
my $db_script = File::Spec->catfile($test_dir, '..', '..', 'sqlite-conf.sql');
if ($ENV{TRAVIS_CI}) {
  $db_script = File::Spec->catfile($test_dir, '..', 'sqlite-conf.sql');
}
$db_script = realpath($db_script);
print STDOUT "# db_script: $db_script\n";

my $conf2sql = File::Spec->catfile($test_dir, '..', '..', 'conf2sql.pl');
$conf2sql = realpath($conf2sql);
print STDOUT "# conf2sql: $conf2sql\n";

my ($ex, $res);
my $db_file = "$tmpdir/proftpd.db";
my $cmd = "sqlite3 $db_file < $db_script";
eval { $res = run_cmd($cmd) };
$ex = $@ if $@;
ok($res && !defined($ex), "built SQLite database");

my $simple_url = "sql://$db_file";
$cmd = "$proftpd -td10 -c '$simple_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "read empty config from simple SQLite URL");

my $complex_url = "sql://$db_file?database=proftpd&ctx=ftpctx:id,parent_id,key,value&map=ftpmap:conf_id,ctx_id&conf=ftpconf:id,key,value";
$cmd = "$proftpd -td10 -c '$complex_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "read empty config from complex SQLite URL");

my $bad_url = "sql://$db_file?database=proftpd&ctx=ftpconf_ctx:id,parent_id,type,value&map=ftpconf_map:conf_id,ctx_id&conf=ftpconf_conf:id,type,value";
$cmd = "$proftpd -td10 -c '$bad_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok(defined($ex), "failed to handle invalid SQLite URL");

# XXX Use conf2sql.pl on the sample-configurations/basic.conf file to generate
# the SQL script to use.
my $config_script = "$tmpdir/config.sql";

$cmd = "sqlite3 $db_file < $config_script";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "populated SQLite database");

$cmd = "$proftpd -td10 -c '$simple_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "read valid config from simple SQLite URL");

$cmd = "$proftpd -td10 -c '$complex_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "read valid config from complex SQLite URL");

# XXX Last, empty/restore the db file, and populate it with BAD config

sub run_cmd {
  my $cmd = shift;
  my $check_exit_status = shift;
  $check_exit_status = 0 unless defined $check_exit_status;

  if ($ENV{TEST_VERBOSE}) {
    print STDOUT "# Executing: $cmd\n";
  }

  my @output = `$cmd > /dev/null`;
  my $exit_status = $?;

  if ($ENV{TEST_VERBOSE}) {
    print STDOUT "# Output: ", join('', @output), "\n";
  }

  if ($check_exit_status) {
    if ($? != 0) {
      croak("'$cmd' failed with exit code $?");
    }
  }

  return 1;
}
