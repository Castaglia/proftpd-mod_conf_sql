#!/usr/bin/env perl

use strict;

use Carp;
use Cwd qw(abs_path realpath);
use File::Path qw(mkpath rmtree);
use File::Spec;
use Test::Simple tests => 1;

my $tmpdir = $ARGV[0];
my $proftpd = $ENV{PROFTPD_TEST_BIN};
my $config_file = "$ENV{TRAVIS_BUILD_DIR}/proftpd/sample-configurations/basic.conf";

my $test_dir = (File::Spec->splitpath(abs_path(__FILE__)))[1];
my $db_script = File::Spec->catfile($test_dir, '..', '..', 'mysql-conf.sql');
if ($ENV{TRAVIS_CI}) {
  $db_script = File::Spec->catfile($test_dir, '..', 'mysql-conf.sql');
}
$db_script = realpath($db_script);

my $conf2sql = File::Spec->catfile($test_dir, '..', '..', 'conf2sql.pl');
$conf2sql = realpath($conf2sql);

ok(1, "no MySQL tests yet");

=pod
my ($ex, $res);
my $db_file = "$tmpdir/proftpd.db";
my $cmd = "sqlite3 $db_file < $db_script";
eval { $res = run_cmd($cmd) };
$ex = $@ if $@;
ok($res && !defined($ex), "built MySQL database");

my $simple_url = "sql://$db_file";
$cmd = "$proftpd -td10 -c '$simple_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "read empty config from simple MySQL URL");

$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "read empty config from simple MySQL URL again");

my $complex_url = "sql://$db_file?ctx=ftpctx:id,parent_id,type,value&map=ftpmap:conf_id,ctx_id&conf=ftpconf:id,name,value";
$cmd = "$proftpd -td10 -c '$complex_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "read empty config from complex MySQL URL");

my $bad_url = "sql://$db_file?ctx=ftpconf_ctx:id,parent_id,type,value&map=ftpconf_map:conf_id,ctx_id&conf=ftpconf_conf:id,type,value";
$cmd = "$proftpd -td10 -c '$bad_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok(defined($ex), "handled invalid MySQL URL");

my $verbose = '';
if ($ENV{TEST_VERBOSE}) {
  $verbose = '--verbose';
}
$cmd = "$conf2sql $verbose --dbdriver=mysql --dbname=$db_file $config_file";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "populated MySQL database");

$cmd = "$proftpd -td10 -c '$simple_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "read valid config from simple MySQL URL");

$cmd = "$proftpd -td10 -c '$complex_url'";
$ex = undef;
eval { $res = run_cmd($cmd, 1) };
$ex = $@ if $@;
ok($res && !defined($ex), "read valid config from complex MySQL URL");

# XXX Last, empty/restore the db file, and populate it with BAD config

=cut

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
