<?php

/**
 * @file
 * Features support for Domain Conf.
 */

/**
 * Implements hook_features_export_options().
 */
function domain_fields_features_export_options() {
  //List for components
  foreach (domain_domains() as $domain) {
    $options[$domain['machine_name']] = $domain['subdomain'];
  }
  return $options;
}

/**
 * Implements hook_features_export().
 */
function domain_fields_features_export($data, &$export, $module_name) {
  $export['dependencies']['domain_fields'] = 'domain_fields';
  $export['dependencies']['domain'] = 'domain';
  $export['dependencies']['features'] = 'features';
  $list = domain_features_selection($data);
  foreach ($list as $machine_name) {
    $all_fields = domain_fields_settings(array('machine_name' => $machine_name));
    if ($all_fields == array())
      continue;
    $export['features']['domain_fields'][$machine_name] = $machine_name;
  }
  return array();
}

/**
 * Implements hook_features_export_render().
 */
function domain_fields_features_export_render($module_name, $data, $export = NULL) {
  domain_fields_features_load($module_name, 'domain_fields_variables', FALSE);
  $code = array();
  $code[] = '  $domain_fields_variables = array();';
  //Create code
  foreach ($data as $key => $machine_name) {
    $all_fields = domain_fields_settings(array('machine_name' => $machine_name));
    if ($all_fields == array())
      continue;
    $entity = $all_fields['node'];
    $code[] = "  \$domain_fields_variables['{$machine_name}'] = array(";
    foreach ($entity as $bundle => $fields) {
      $code[] = "    '{$bundle}' => array(";
      $flat = FALSE;
      foreach ($fields as $field_name => $options) {
        if (array_search('1', $options)) {
          $code[] = "      '{$field_name}' => " . features_var_export($options, '      ') . ",";
          $flat = TRUE;
        }
      }
      if (!$flat) {
        //Delete bundles null
        array_pop($code);
      } else {
        $code[] = "    ),";
      }
    }
    $code[] = "  );";
  }
  $code[] = "\n  return \$domain_fields_variables;";
  $output = implode("\n", $code);
  return array('domain_fields_variables' => $output);
}

/**
 * Implements hook_features_revert().
 */
function domain_fields_features_revert($module) {
  return domain_fields_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild().
 */
function domain_fields_features_rebuild($module) {
  if ($domain_fields_variables = domain_fields_features_load($module, 'domain_fields_variables', TRUE)) {
    foreach ($domain_fields_variables as $machine_name => $feature_values) {
      $settings = variable_domain_fields_get('domain_fields_' . $machine_name, array());
      foreach ($feature_values as $bundle => $field_values) {
        foreach ($field_values as $field_name => $values) {

          if (isset($settings['node'][$bundle][$field_name])) {
            $settings['node'][$bundle][$field_name] = array_merge($settings['node'][$bundle][$field_name], $values);
          }
        }
      }
      variable_domain_fields_set('domain_fields_' . $machine_name, $settings);
    }
  }
}

/**
 * Features doesn't know how to load custom includes.
 *
 * @param $module
 *  The name of the feature to load.
 * @param $hook
 *  The name of the domain hook.
 * @param $return
 *  Boolean indicator to return the results of the function.
 *
 * @return
 *  The results of the $hook implemenation, if requested.
 */
function domain_fields_features_load($module, $hook, $return = TRUE) {
  // Features does not handle module loading of custom files.
  module_load_include('inc', $module, $module . '.fields_export');
  $function = $module . '_' . $hook;
  if ($return && function_exists($function)) {
    return $function();
  }
}