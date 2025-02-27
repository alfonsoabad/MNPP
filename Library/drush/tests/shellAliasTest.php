<?php

/*
 * @file
 *   Tests for Shell aliases.
 */
class shellAliasesCase extends Drush_CommandTestCase {

  /**
   * Write a config file that contains the shell-aliases array.
   */
  static function setupBeforeClass() {
    parent::setUpBeforeClass();
    $contents = "
      <?php

      \$options['shell-aliases'] = array(
        'glopts' => 'topic core-global-options',
        'pull' => '!git pull',
        'echosimple' => '!echo {{@target}}',
        'echotest' => '!echo {{@target}} {{%root}} {{%mypath}}',
        'compound-command' => '!cd {{%sandbox}} && pwd && touch mytest && ls mytest',
      );
    ";
    file_put_contents(UNISH_SANDBOX . '/drushrc.php', trim($contents));
    mkdir(UNISH_SANDBOX . '/b');
    $contents = "
      <?php

      \$options['shell-aliases'] = array(
        'also' => '!echo alternate config file included too',
      );
    ";
    file_put_contents(UNISH_SANDBOX . '/b/drushrc.php', trim($contents));
    $contents = "
      <?php

      \$aliases['myalias'] = array (
        'root' => '/path/to/drupal',
        'uri' => 'mysite.org',
        '#peer' => '@live',
        'path-aliases' => array (
          '%mypath' => '/srv/data/mypath',
          '%sandbox' => '" . UNISH_SANDBOX . "'
        ),
      );
    ";
    file_put_contents(UNISH_SANDBOX . '/aliases.drushrc.php', trim($contents));
  }

  /*
   * Test shell aliases to Drush commands.
   */
  public function testShellAliasDrushLocal() {
    $options = array(
      'config' => UNISH_SANDBOX,
    );
    $this->drush('glopts', array(), $options);
    $output = $this->getOutput();
    $this->assertContains('These options are applicable to most drush commands.', $output, 'Successfully executed local shell alias to drush command');
  }

  /*
   * Test shell aliases to Bash commands. Assure we pass along extra arguments
   * and options.
   */
  public function testShellAliasBashLocal() {
    $options = array(
      'config' => UNISH_SANDBOX,
      'simulate' => NULL,
      'rebase' => NULL,
    );
    $this->drush('pull', array('origin'), $options);
    $output = $this->getOutput();
    $this->assertContains('Calling proc_open(git pull origin --rebase);', $output);
  }

  public function testShellAliasDrushRemote() {
    $options = array(
      'config' => UNISH_SANDBOX,
      'simulate' => NULL,
      'ssh-options' => '',
    );
    $this->drush('glopts', array(), $options, 'user@server/path/to/drupal#sitename');
    // $expected might be different on non unix platforms. We shall see.
    $expected = "Simulating backend invoke: ssh user@server 'drush  --simulate --nocolor --uri=sitename --root=/path/to/drupal --config=/tmp/drush-sandbox topic core-global-options --invoke 2>&1' 2>&1";
    $output = $this->getOutput();
    $this->assertEquals($expected, $output, 'Expected remote shell alias to a drush command was built');
  }

  public function testShellAliasBashRemote() {
    $options = array(
      'config' => UNISH_SANDBOX,
      'simulate' => NULL,
      'ssh-options' => '',
      'rebase' => NULL,
    );
    $this->drush('pull', array('origin'), $options, 'user@server/path/to/drupal#sitename');
    // $expected might be different on non unix platforms. We shall see.
    $expected = "Calling proc_open(ssh  user@server 'cd /path/to/drupal && git pull origin --rebase');";
    $output = $this->getOutput();
    $this->assertEquals($expected, $output, 'Expected remote shell alias to a bash command was built');
  }
  
  /*
   * Test shell aliases with simple replacements -- no alias.
   */
  public function testShellAliasSimpleReplacement() {
    $options = array(
      'config' => UNISH_SANDBOX,
    );
    $this->drush('echosimple', array(), $options);
    $expected = "@none";
    $output = $this->getOutput();
    $this->assertEquals($expected, $output, 'Expected echo test returned "@none"');
  }
  
  /*
   * Test shell aliases with complex replacements -- no alias.
   */
  public function testShellAliasReplacementNoAlias() {
    $options = array(
      'config' => UNISH_SANDBOX,
    );
    // echo test has replacements that are not satisfied, so this is expected to return an error.
    $this->drush('echotest', array(), $options, NULL, NULL, self::EXIT_ERROR);
  }
  
  /*
   * Test shell aliases with replacements -- alias.
   */
  public function testShellAliasReplacementWithAlias() {
    $options = array(
      'config' => UNISH_SANDBOX,
      'alias-path' => UNISH_SANDBOX,
    );
    $this->drush('echotest', array(), $options, '@myalias');
    $expected = "@myalias /path/to/drupal /srv/data/mypath";
    $output = $this->getOutput();
    $this->assertEquals($expected, $output, 'Expected echo test returned "' . $expected . '"');
  }
  
  /*
   * Test shell aliases with replacements and compound commands.
   */
  public function testShellAliasCompoundCommands() {
    $options = array(
      'config' => UNISH_SANDBOX,
      'alias-path' => UNISH_SANDBOX,
    );
    $this->drush('compound-command', array(), $options, '@myalias');
    $expected = UNISH_SANDBOX . "\nmytest";
    $output = $this->getOutput();
    $this->assertEquals($expected, $output, 'Expected compound test returned "' . $expected . '"');
  }

  
  /*
   * Test shell aliases with replacements and compound commands.
   */
  public function testShellAliasMultipleConfigFiles() {
    $options = array(
      'config' => UNISH_SANDBOX . "/b" . PATH_SEPARATOR . UNISH_SANDBOX,
      'alias-path' => UNISH_SANDBOX,
    );
    $this->drush('compound-command', array(), $options, '@myalias');
    $expected = UNISH_SANDBOX . "\nmytest";
    $output = $this->getOutput();
    $this->assertEquals($expected, $output, 'Expected compound test returned "' . $expected . '"');
    $this->drush('also', array(), $options);
    $expected = "alternate config file included too";
    $output = $this->getOutput();
    $this->assertEquals($expected, $output, 'Expected also test returned "' . $expected . '"');
  }

}

