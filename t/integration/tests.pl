#!/usr/bin/env perl

use strict;

use Cwd qw(abs_path realpath);
use File::Path qw(mkpath rmtree);
use File::Spec;
use File::Temp qw(tempdir);
use Getopt::Long;
use TAP::Harness;

my $testno = 0;

my $opts = {};
GetOptions($opts, 'h|help', 'V|verbose');

usage() if $opts->{h};
$ENV{TEST_VERBOSE} = 1 if $opts->{V};
$opts->{V} = 1 if $ENV{TEST_VERBOSE};

my $test_dir = (File::Spec->splitpath(abs_path(__FILE__)))[1];

# This is to handle the case where this tests.pl script might be
# being used to run test files other than those that ship with proftpd,
# e.g. to run the tests that come with third-party modules.
unless (defined($ENV{PROFTPD_TEST_BIN})) {
  my $bin = File::Spec->catfile($test_dir, '..', '..', 'proftpd');
  if ($ENV{TRAVIS_CI}) {
    $bin = "$ENV{TRAVIS_BUILD_DIR}/proftpd/proftpd";
  }
  $ENV{PROFTPD_TEST_BIN} = realpath($bin);
}

$| = 1;

print STDOUT "# PROFTPD_TEST_BIN = $ENV{PROFTPD_TEST_BIN}\n";

my $test_files = [
  ["$test_dir/sqlite.t", 'sqlite'],
];

# Create a temp directory for each separate test, pass it in, cleanup afterward
my $tap_test_args = {
  'sqlite' => [get_tmp_dir()],
};

my $tap_opts = {
  verbosity => 0,
  test_args => $tap_test_args,
};

if ($opts->{V}) {
  $tap_opts->{verbosity} = 1;
}

my $harness = TAP::Harness->new($tap_opts);
my $aggregator = $harness->runtests(@$test_files);
print STDOUT "Integration tests: ", $aggregator->get_status(), "\n";

# Cleanup
foreach my $alias (keys(%$tap_test_args)) {
  my $tmpdir = $tap_test_args->{$alias}->[0];
  rmtree($tmpdir);
}

exit 0 if $aggregator->all_passed();
exit 1;

sub get_tmp_dir {
  ++$testno;

  my $tmpdir = tempdir(
    "mod_conf_sql-integration-$$-test$testno-XXXXXXXXXX",
    TMPDIR => 1,
    CLEANUP => 0,
  );

  unless (chmod(0755, $tmpdir)) {
    croak("Can't chmod tmp directory $tmpdir: $!");
  }

  return $tmpdir;
}

sub usage {
  print STDOUT <<EOH;

$0: [--help] [--verbose]

Examples:

  \$ perl $0
  \$ perl $0 --verbose

EOH
  exit 0;
}
