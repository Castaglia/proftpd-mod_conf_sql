#!/usr/bin/env perl

use strict;

use Cwd qw(abs_path realpath);
use File::Path qw(mkpath rmtree);
use File::Spec;
use Test::Simple tests => 1;

my $tmpdir = $ARGV[0];
print STDOUT "# tmpdir: $tmpdir\n";

my $test_dir = (File::Spec->splitpath(abs_path(__FILE__)))[1];
my $db_script = File::Spec->catfile($test_dir, '..', 'sqlite-conf.sql');
$db_script = realpath($db_script);
print STDOUT "# db_script: $db_script\n";

my $db_file = "$tmpdir/proftpd.db";
my $cmd = "sqlite3 $db_file < $db_script";
my $res = build_db($cmd);
ok($res, "build SQLite database");

# Find proftpd binary, run "proftpd -td10 -c $url"

sub build_db {
  my $cmd = shift;
  my $check_exit_status = shift;
  $check_exit_status = 0 unless defined $check_exit_status;

  if ($ENV{TEST_VERBOSE}) {
    print STDOUT "# Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  my $exit_status = $?;

  if ($ENV{TEST_VERBOSE}) {
    print STDOUT "# Output: ", join('', @output), "\n";
  }

  if ($check_exit_status) {
    if ($? != 0) {
      croak("'$cmd' failed");
    }
  }

  return 1;
}
