#!/usr/bin/env perl

use strict;

use File::Path qw(mkpath rmtree);
use File::Spec;
use Test::Simple tests => 1;

my $tmpdir = $ARGV[0];
print STDOUT "# tmpdir: $tmpdir\n";

my $test_dir = (File::Spec->splitpath(abs_path(__FILE__)))[1];
my $script = File::Spec->catfile($test_dir, '..', '..', 'sqlite-conf.sql');
$script = File::Spec->canonpath($script);
print STDOUT "# script: $script\n";

ok(1 + 1 == 2);

# Find sqlite-conf.sql from current directory.
# Run sqlite3 exec to create "$tmpdir/proftpd.db"
# Find proftpd binary, run "proftpd -td10 -c $url"
# Cleanup "$tmpdir/proftpd.db"
