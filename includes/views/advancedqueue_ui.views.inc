<?php

/**
 * Implements hook_views_data_alter().
 *
 * @see advancedqueue_views_data()
 */
function advancedqueue_ui_views_data_alter(&$data) {
  // Add string filter for the Result field.
  $data['advancedqueue']['result']['filter']['handler'] = 'views_handler_filter_string';

  // Use custom handler extending views_handler_field_serialized to avoid
  // 'Undefined index' PHP notices if the value does not exist in the serialized
  // Data field.
  $data['advancedqueue']['data']['field']['handler'] = 'advancedqueue_ui_handler_field_serialized';
  // Add string filter for the Data field.
  $data['advancedqueue']['data']['filter']['handler'] = 'views_handler_filter_string';
}

/**
 * Implements hook_views_plugins().
 *
 * Based on comment #11855075 in issue #1362524 "add GROUP_CONCAT aggregate
 * function; and move aggregate function helpers to a query extender?"
 *
 * @see https://www.drupal.org/node/1362524#comment-11855075
 */
function advancedqueue_ui_views_plugins() {
  return array(
    'query_aggregate' => array(
      'group_concat_distinct' => array(
        'title' => t('Group concat DISTINCT'),
        'method' => 'advancedqueue_ui_method_group_concat_distinct',
        'handler' => array(
          'argument' => 'views_handler_argument_group_by_numeric',
          'filter' => 'views_handler_filter_group_by_numeric',
          'sort' => 'views_handler_sort_group_by_numeric',
        ),
        'file' => 'includes/views/advancedqueue_ui.views.inc',
      ),
    ),
  );
}

/**
 * Callback function for the GROUP_CONCAT DISTINCT views aggregation group type.
 *
 * @see advancedqueue_ui_views_plugins()
 */
function advancedqueue_ui_method_group_concat_distinct($group_type, $field) {
  return "GROUP_CONCAT(DISTINCT NULLIF($field, '') SEPARATOR ', ')";
}

/**
 * Implements hook_views_pre_build().
 *
 * Hides 'Queue name' column and filter on Queue items pages
 * when the provided queue name argument is different from 'all'.
 */
function advancedqueue_ui_views_pre_build(&$view) {
  if ($view->name == 'advancedqueue_ui' && $view->args[0] != 'all') {
    unset($view->field['queue_name']);
    unset($view->field['queue_machine_name']);
    unset($view->filter['combine_name_machine_name']);
  }
}

/**
 * Implements hook_views_post_build().
 *
 * Hides the whole VBO form from the queue items view
 * if current user does not have access to manage queues.
 *
 * @see views_bulk_operations_views_post_build()
 */
function advancedqueue_ui_views_post_build(&$view) {
  if ($view->name == 'advancedqueue_ui' && !user_access('advancedqueue_ui manage queues')) {
    $vbo = _views_bulk_operations_get_field($view);
    $vbo->options['exclude'] = TRUE;
  }
}
