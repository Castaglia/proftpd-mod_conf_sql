#!/usr/bin/env perl

use strict;

use Cwd qw(abs_path);
use File::Spec;
use Getopt::Long;
use TAP::Harness;

my $opts = {};
GetOptions($opts, 'h|help', 'V|verbose');

usage() if $opts->{h};
$ENV{TEST_VERBOSE} = 1 if $opts->{V};

my $test_dir = (File::Spec->splitpath(abs_path(__FILE__)))[1];

# This is to handle the case where this tests.pl script might be
# being used to run test files other than those that ship with proftpd,
# e.g. to run the tests that come with third-party modules.
unless (defined($ENV{PROFTPD_TEST_BIN})) {
  $ENV{PROFTPD_TEST_BIN} = File::Spec->catfile($test_dir, '..', 'proftpd');
}

$| = 1;

my $test_files;
if (scalar(@ARGV) > 0) {
  $test_files = [@ARGV];

} else {
  $test_files = [qw(
    t/integration/sqlite.t
  )];
}

my $tap_opts = {
  verbosity => 0,
};

if ($opts->{V}) {
  $tap_opts->{verbosity} = 1;
}

my $harness = TAP::Harness->new($tap_opts);
$harness->runtests(@$test_files) if scalar(@$test_files) > 0;

exit 0;

sub usage {
  print STDOUT <<EOH;

$0: [--help] [--verbose]

Examples:

  \$ perl $0
  \$ perl $0 --verbose

EOH
  exit 0;
}
