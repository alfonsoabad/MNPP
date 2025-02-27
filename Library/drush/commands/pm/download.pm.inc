<?php

/**
 * Implementation of drush_COMMAND_validate().
 */
function drush_pm_download_validate() {
  // Validate the user specified destination directory.
  $destination = drush_get_option('destination');
  if (!empty($destination)) {
    $destination = rtrim($destination, DIRECTORY_SEPARATOR);
    if (!is_dir($destination)) {
      drush_print(dt("The directory !destination does not exist.", array('!destination' => $destination)));
      if (!drush_get_context('DRUSH_SIMULATE')) {
        if (drush_confirm(dt('Would you like to create it?'))) {
          drush_mkdir($destination);
        }
        if (!is_dir($destination)) {
          return drush_set_error('DRUSH_PM_NO_DESTINATION', dt('Unable to create destination directory !destination.', array('!destination' => $destination)));
        }
      }
    }
    if (!is_writable($destination)) {
      return drush_set_error('DRUSH_PM_NO_DESTINATION', dt('Destination directory !destination is not writable.', array('!destination' => $destination)));
    }
    // Ignore --use-site-dir, if given.
    if (drush_get_option('use-site-dir', FALSE)) {
      drush_set_option('use-site-dir', FALSE);
    }
  }

  // Validate --variant or enforce a sane default.
  $variant = drush_get_option('variant', FALSE);
  if ($variant) {
    if (!in_array($variant, array('full', 'projects', 'profile-only'))) {
      drush_log(dt('Unknown variant !variant. Valid values: !variations', array('!variant' => $variant, '!variations' => implode(', ', $variations))), 'error');
    }
  }
  // 'full' and 'projects' variants are only valid for wget package handler.
  $package_handler = drush_get_option('package-handler', 'wget');
  if (($package_handler != 'wget') && ($variant != 'profile-only')) {
    $new_variant = 'profile-only';
    if ($variant) {
      drush_log(dt('Variant !variant is incompatible with !ph package-handler.', array('!variant' => $variant, '!ph' => $package_handler)), 'warning');
    }
  }
  // If we are working on a drupal root, full variant is not an option.
  else if (drush_get_context('DRUSH_BOOTSTRAP_PHASE') >= DRUSH_BOOTSTRAP_DRUPAL_ROOT) {
    if ((!$variant) || (($variant == 'full') && (!isset($new_variant)))) {
      $new_variant = 'projects';
    }
    if ($variant == 'full') {
      drush_log(dt('Variant full is not a valid option within a Drupal root.'), 'warning');
    }
  }

  if (isset($new_variant)) {
    drush_set_option('variant', $new_variant);
    if ($variant) {
      drush_log(dt('Switching to --variant=!variant.', array('!variant' => $new_variant)), 'ok');
    }
  }

  // Validate package-handler.
  $package_handler = drush_get_option('package-handler', 'wget');
  drush_include_engine('package_handler', $package_handler);
  // Return value not currently used.
  return package_handler_validate();
}

/**
 * Command callback. Download Drupal core or any project.
 */
function drush_pm_download() {
  $package_handler = drush_get_option('package-handler', 'wget');
  drush_include_engine('package_handler', $package_handler);
  drush_include_engine('release_info', 'updatexml');

  if (!$requests = pm_parse_arguments(func_get_args(), FALSE)) {
    $requests = array('drupal');
  }

  // Parse out project name and version.
  $requests = pm_parse_project_version($requests);

  // Get release history for each request and download the project.
  foreach ($requests as $name => $request) {
    $release = release_info_fetch($request);
    if ($release == FALSE) {
      continue;
    }

    // Download the project to a temporary location.
    $request['base_project_path'] = drush_tempdir();
    $request['full_project_path'] = $request['base_project_path'] .'/'. $request['project_dir'];
    drush_log(dt('Downloading project !name to !dir ...', array('!name' => $request['name'], '!dir' => $request['base_project_path'])));
    if (!package_handler_download_project($request, $release)) {
      drush_log(dt('Error downloading !name', array('!name' => $request['name']), 'error'));
      continue;
    }

    // Determine the install location for the project.  User provided
    // --destination has preference.
    $destination = drush_get_option('destination');
    if (!empty($destination)) {
      $request['project_install_location'] = realpath($destination);
    }
    else {
      $request['project_install_location'] = _pm_download_destination($request['project_type']);
    }

    // If user did not provide --destination, then call the
    // download-destination-alter hook to give the chance to any commandfiles
    // to adjust the install location or abort it.
    if (empty($destination)) {
      $result = drush_command_invoke_all_ref('drush_pm_download_destination_alter', $request, $release);
      if (array_search(FALSE, $result, TRUE) !== FALSE) {
        return FALSE;
      }
    }

    // Load version control engine and detect if (the parent directory of) the
    // project install location is under a vcs.
    if (!$version_control = drush_pm_include_version_control($request['project_install_location'])) {
      continue;
    }

    // Check for drush self update
    if ($request['project_install_location'] == DRUSH_BASE_PATH && $request['name'] == 'drush') {
      if (($backup_dir = drush_prepare_backup_dir()) === FALSE) {
        return FALSE;
      }
      // Move the running drush out of the way
      $drush_backup = $backup_dir . "/drush";
      if (drush_move_dir(DRUSH_BASE_PATH, $drush_backup, TRUE) == FALSE) {
        return drush_set_error('DRUSH_PM_BACKUP_FAILED', dt('Failed to move drush directory !drush to !backup_target', array('!drush' => DRUSH_BASE_PATH, '!backup_target' => $drush_backup)));
      }
      else {
        drush_log(dt("drush backed up to !targetdir", array('!targetdir' => $drush_backup)), "ok");
      }
    }
    else {
      // For all other projects, the final project install location will go in the project_dir.
      $request['project_install_location'] .= '/' . $request['project_dir'];
    }

    if ($version_control->engine == 'backup') {
      // Check if install location already exists.
      if (is_dir($request['project_install_location'])) {
        if (drush_confirm(dt('Install location !location already exists. Do you want to overwrite it?', array('!location' => $request['project_install_location'])))) {
          drush_delete_dir($request['project_install_location'], TRUE);
        }
        else {
          drush_log(dt("Skip installation of !project to !dest.", array('!project' => $request['name'], '!dest' => $request['project_install_location'])), 'warning');
          continue;
        }
      }
    }
    else {
      // Find and unlink all files but the ones in the vcs control directories.
      $skip_list = array('.', '..');
      $skip_list = array_merge($skip_list, drush_version_control_reserved_files());
      drush_scan_directory($request['project_install_location'], '/.*/', $skip_list, 'unlink', TRUE, 'filename', 0, TRUE);
    }

    // Copy the project to the install location.
    if (drush_op('_drush_recursive_copy', $request['full_project_path'], $request['project_install_location'])) {
      drush_log(dt("Project !project (!version) downloaded to !dest.", array('!project' => $request['name'], '!version' => $release['version'], '!dest' => $request['project_install_location'])), 'success');
      $request['base_project_path'] = basename($request['project_install_location']);
      $request['full_project_path'] = $request['project_install_location'];
      if ($request['project_install_location'] == DRUSH_BASE_PATH) {
        drush_log(dt("Drush successfully updated to version !version.", array('!version' => $release['version'])), 'success');
      }
      // If the version control engine is a proper vcs we also need to remove
      // orphan directories.
      if ($version_control->engine != 'backup') {
        $empty_dirs = drush_find_empty_directories($request['full_project_path'], $version_control->reserved_files());
        foreach ($empty_dirs as $empty_dir) {
          // Some VCS files are read-only on Windows (e.g., .svn/entries).
          drush_delete_dir($empty_dir, TRUE);
        }
      }
    }
    else {
      drush_log(dt("Project !project (!version) could not be downloaded to !dest.", array('!project' => $request['name'], '!version' => $release['version'], '!dest' => $request['project_install_location'])), 'error');
      continue;
    }

    // Post download actions.
    package_handler_post_download($request);
    drush_command_invoke_all('drush_pm_post_download', $request, $release);
    $version_control->post_download($request);

    // Print release notes if --notes option is set.
    if (drush_get_option('notes') && !drush_get_context('DRUSH_PIPE')) {
      _drush_pm_releasenotes(array($name . '-' . $release['version']), FALSE);
    }

    // Inform the user about available modules a/o themes in the downloaded project.
    drush_pm_extensions_in_project($request);
  }
}

/**
 * Return an array of available releases for given project(s).
 *
 * Helper function for pm-download.
 */
function _pm_download_releases_choice($xml, $project, $all = FALSE, $dev = FALSE) {
  $project_info = updatexml_get_releases_from_xml($xml, $project);
  $releases = _drush_pm_filter_releases($project_info['releases'], $all, $dev);

  $options = array();
  foreach($releases as $version => $release) {
    $options[$version] = array($version, '-', gmdate('Y-M-d', $release['date']), '-', implode(', ', $release['release_status']));
  }
  return $options;
}

/**
 * Implementation of hook_drush_pm_download_destination_alter().
 *
 * Built-in download-destination-alter hook. This particular version of
 * the hook will move modules that contain only drush commands to
 * /usr/share/drush/commands if it exists, or $HOME/.drush if the
 * site-wide location does not exist.
 */
function pm_drush_pm_download_destination_alter(&$project, $release) {
  // A module is a pure drush command if it has no .module and contain
  // .drush.inc files.
  if ($project['project_type'] == 'module') {
    $module_files = drush_scan_directory($project['full_project_path'], '/.*\.module/');
    if (empty($module_files)) {
      $drush_command_files = drush_scan_directory($project['full_project_path'], '/.*\.drush.inc/');
      if (!empty($drush_command_files)) {
        $install_dir = drush_get_context('SHARE_PREFIX', '/usr') . '/share/drush/commands';
        if (!is_dir($install_dir) || !is_writable($install_dir)) {
          $install_dir = drush_server_home() . '/.drush';
        }
        // Make the .drush dir if it does not already exist.
        if (!is_dir($install_dir)) {
          drush_mkdir($install_dir);
        }
        // Change the location if the mkdir worked.
        if (is_dir($install_dir)) {
          $project['project_install_location'] = $install_dir;
          // We need to clear the drush commandfile cache so that
          // our newly-downloaded drush extension commandfiles can be found.
          drush_cache_clear_all();
        }
      }
    }
  }
}

/**
 * Determine a candidate destination directory for a particular site path and
 * return it if it exists, optionally attempting to create the directory.
 */
function _pm_download_destination_lookup($type, $drupal_root, $sitepath, $create = FALSE) {
  switch ($type) {
    case 'module':
      // Prefer sites/all/modules/contrib if it exists.
      $destination = $sitepath . '/modules';
      $contrib = $destination . '/contrib';
      if (is_dir($contrib)) {
        $destination = $contrib;
      }
      break;
    case 'theme':
      $destination = $sitepath . '/themes';
      break;
    case 'theme engine':
      $destination = $sitepath . '/themes/engines';
      break;
    case 'profile':
      $destination = $drupal_root . '/profiles';
      break;
  }
  if ($create) {
    drush_log(dt('Attempting to create destination directory at !dir', array('!dir' => $destination)));
    drush_mkdir($destination);
  }
  if (is_dir($destination)) {
    drush_log(dt('Using destination directory !dir', array('!dir' => $destination)));
    return $destination;
  }
  drush_log(dt('Could not find destination directory at !dir', array('!dir' => $destination)));
  return FALSE;
}

/**
 * Returns the best destination for a particular download type we can find.
 *
 * It is based on the project type and drupal and site contexts.
 */
function _pm_download_destination($type) {
  $drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT');
  $site_root = drush_get_context('DRUSH_DRUPAL_SITE_ROOT', FALSE);
  $full_site_root = $drupal_root .'/'. $site_root;
  $sites_all = $drupal_root . '/sites/all';

  $in_site_directory = FALSE;
  // Check if we are running within the site directory.
  if ($full_site_root == substr(drush_cwd(), 0, strlen($full_site_root)) || (drush_get_option('use-site-dir', FALSE))) {
    $in_site_directory = TRUE;
  }

  $destination = '';
  if ($type != 'core') {
    // Attempt 1: If we are in a specific site directory, and the destination
    // directory already exists, then we use that.
    if (empty($destination) && $site_root && $in_site_directory) {
      $create_dir = drush_get_option('use-site-dir', FALSE);
      $destination = _pm_download_destination_lookup($type, $drupal_root, $full_site_root, $create_dir);
    }
    // Attempt 2: If the destination directory already exists for sites/all,
    // then we use that.
    if (empty($destination) && $drupal_root) {
      $destination = _pm_download_destination_lookup($type, $drupal_root, $sites_all);
    }
    // Attempt 3: If a specific (non default) site directory exists and
    // sites/all does not exist, then we create destination in the site
    // specific directory.
    if (empty($destination) && $site_root && $site_root !== 'sites/default' && is_dir($full_site_root) && !is_dir($sites_all)) {
      $destination = _pm_download_destination_lookup($type, $drupal_root, $full_site_root, TRUE);
    }
    // Attempt 4: If sites/all exists, then we create destination in the
    // sites/all directory.
    if (empty($destination) && is_dir($sites_all)) {
      $destination = _pm_download_destination_lookup($type, $drupal_root, $sites_all, TRUE);
    }
    // Attempt 5: If site directory exists (even default), then we create
    // destination in the this directory.
    if (empty($destination) && $site_root && is_dir($full_site_root)) {
      $destination = _pm_download_destination_lookup($type, $drupal_root, $full_site_root, TRUE);
    }
  }
  // Attempt 6: If we didn't find a valid directory yet (or we somehow found
  // one that doesn't exist) we always fall back to the current directory.
  if (empty($destination) || !is_dir($destination)) {
    $destination = drush_cwd();
  }

  return $destination;
}
