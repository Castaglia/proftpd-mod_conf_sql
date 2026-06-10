package ProFTPD::Tests::Modules::mod_conf_sql;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use Carp;
use File::Path qw(mkpath);
use File::Spec;
use IO::Handle;
use Time::HiRes qw(usleep);

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  conf_sql_sqlite_empty_config_simple_url_syntax_check => {
    order => ++$order,
    test_class => [qw(forking mod_sql mod_sql_sqlite)],
  },

  conf_sql_sqlite_empty_config_complex_url_syntax_check => {
    order => ++$order,
    test_class => [qw(forking mod_sql mod_sql_sqlite)],
  },

  conf_sql_sqlite_full_config_simple_url_syntax_check => {
    order => ++$order,
    test_class => [qw(forking mod_sql mod_sql_sqlite)],
  },

  conf_sql_sqlite_full_config_complex_url_syntax_check => {
    order => ++$order,
    test_class => [qw(forking mod_sql mod_sql_sqlite)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  return testsuite_get_runnable_tests($TESTS);
}

# Support functions

sub get_proftpd_bin {
  my $proftpd_bin = $ENV{PROFTPD_TEST_BIN};
  unless ($proftpd_bin) {
    $proftpd_bin = '../../proftpd';
  }

  return $proftpd_bin;
}

sub run_cmd {
  my $cmd = shift;
  my $check_exit_status = shift;
  $check_exit_status = 0 unless defined $check_exit_status;

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing: $cmd\n";
  }

  my @output = `$cmd`;
  my $exit_status = $?;

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  if ($check_exit_status) {
    if ($ENV{TEST_VERBOSE}) {
      print STDERR "Exit status: $exit_status\n";
    }

    if ($exit_status != 0) {
      croak("'$cmd' failed");
    }
  }

  return 1;
}

# Test cases

sub conf_sql_sqlite_empty_config_simple_url_syntax_check {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'conf_sql');

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");
  my $db_script = File::Spec->rel2abs('t/etc/modules/mod_conf_sql/sqlite-conf.sql');
  my $cmd = "sqlite3 $db_file < $db_script";
  run_cmd($cmd);

  # Do a config syntax check only
  my $proftpd_bin = get_proftpd_bin();
  my $proftpd_opts = '-q -t';
  my $tracing = 'false';

  if ($ENV{TEST_VERBOSE}) {
    $proftpd_opts = '-td10';
    $tracing = 'true';
  }

  my $url = "sql://$db_file?tracing=$tracing&driver=sqlite";

  my $ex;
  $cmd = "$proftpd_bin $proftpd_opts -c '$url' 2>&1";
  eval { run_cmd($cmd, 1) };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub conf_sql_sqlite_empty_config_complex_url_syntax_check {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'conf_sql');

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");
  my $db_script = File::Spec->rel2abs('t/etc/modules/mod_conf_sql/sqlite-conf.sql');
  my $cmd = "sqlite3 $db_file < $db_script";
  run_cmd($cmd);

  # Do a config syntax check only
  my $proftpd_bin = get_proftpd_bin();
  my $proftpd_opts = '-q -t';
  my $tracing = 'false';

  if ($ENV{TEST_VERBOSE}) {
    $proftpd_opts = '-td10';
    $tracing = 'true';
  }

  my $url = "sql://$db_file?tracing=$tracing&driver=sqlite&ctx=ftpctx:id,parent_id,type,value&map=ftpmap:conf_id,ctx_id&conf=ftpconf:id,name,value";

  my $ex;
  $cmd = "$proftpd_bin $proftpd_opts -c '$url' 2>&1";
  eval { run_cmd($cmd, 1) };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub conf_sql_sqlite_full_config_simple_url_syntax_check {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'conf_sql');

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");
  my $db_script = File::Spec->rel2abs('t/etc/modules/mod_conf_sql/sqlite-conf.sql');
  my $cmd = "sqlite3 $db_file < $db_script";
  run_cmd($cmd);

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'conf_sql:20 sql:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  my $conf2sql = File::Spec->rel2abs('conf2sql.pl');

  # Populate the database with our configuration
  my $verbose = '';
  if ($ENV{TEST_VEROBSE}) {
    $verbose = '--verbose';
  }

  my $ex;
  $cmd = "$conf2sql $verbose --dbdriver=sqlite --dbname=$db_file $setup->{config_file}";
  eval { run_cmd($cmd, 1) };
  if ($@) {
    $ex = $@;
  }

  if ($ex) {
    test_cleanup($setup->{log_file}, $ex);
    return;
  }

  # Do a config syntax check only
  my $proftpd_bin = get_proftpd_bin();
  my $proftpd_opts = '-t';
  my $tracing = 'false';

  if ($ENV{TEST_VERBOSE}) {
    $proftpd_opts = '-td10';
    $tracing = 'true';
  }

  my $url = "sql://$db_file?tracing=$tracing&driver=sqlite";

  $cmd = "$proftpd_bin $proftpd_opts -c '$url'";
  eval { run_cmd($cmd, 1) };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub conf_sql_sqlite_full_config_complex_url_syntax_check {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'conf_sql');

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");
  my $db_script = File::Spec->rel2abs('t/etc/modules/mod_conf_sql/sqlite-conf.sql');
  my $cmd = "sqlite3 $db_file < $db_script";
  run_cmd($cmd);

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'conf_sql:20 sql:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  my $conf2sql = File::Spec->rel2abs('conf2sql.pl');

  # Populate the database with our configuration
  my $verbose = '';
  if ($ENV{TEST_VEROBSE}) {
    $verbose = '--verbose';
  }

  my $ex;
  $cmd = "$conf2sql $verbose --dbdriver=sqlite --dbname=$db_file $setup->{config_file}";
  eval { run_cmd($cmd, 1) };
  if ($@) {
    $ex = $@;
  }

  if ($ex) {
    test_cleanup($setup->{log_file}, $ex);
    return;
  }

  # Do a config syntax check only
  my $proftpd_bin = get_proftpd_bin();
  my $proftpd_opts = '-t';
  my $tracing = 'false';

  if ($ENV{TEST_VERBOSE}) {
    $proftpd_opts = '-td10';
    $tracing = 'true';
  }

  my $url = "sql://$db_file?tracing=$tracing&driver=sqlite&ctx=ftpctx:id,parent_id,type,value&map=ftpmap:conf_id,ctx_id&conf=ftpconf:id,name,value";

  $cmd = "$proftpd_bin $proftpd_opts -c '$url'";
  eval { run_cmd($cmd, 1) };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

1;
