<?php

/**
 * @file
 * Drush worker for Advanced Queues Admin.
 */

/**
 * Implements hook_drush_command().
 */
function advancedqueue_admin_drush_command() {
  $items = array();

  $items['advancedqueue-delete'] = array(
    'description' => 'Deletes all items from a queue.',
    'arguments' => array(
      'queue' => dt('The name of the queue to delete items from.'),
    ),
    'options' => array(
      'all' => dt('Delete all items from all queues.'),
    ),
    'aliases' => array('aqd'),
  );

  $items['advancedqueue-delete-item'] = array(
    'description' => 'Deletes a specific item from a queue.',
    'arguments' => array(
      'queue' => dt('The name of the queue to delete an item from.'),
      'item_id' => dt('The ID of the item to delete.'),
    ),
    'aliases' => array('aqdi'),
  );

  $items['advancedqueue-list-items'] = array(
    'description' => 'Returns a list of unprocessed items in a queue.',
    'arguments' => array(
      'queue' => dt('The name of the queue to return unprocessed items for.'),
    ),
    'options' => array(
      'all' => 'Return unprocessed items for all queues.',
    ),
    'aliases' => array('aqli'),
  );

  return $items;
}

/**
 * Implements hook_drush_command_alter().
 */
function advancedqueue_admin_drush_command_alter(&$command) {
  if ($command->command == 'advancedqueue-process-queue') {
    $command->aliases[] = 'aqp';
  }
  if ($command->command == 'advancedqueue-list') {
    $command->aliases[] = 'aql';
  }
}

/**
 * Command callback for drush advancedqueue-delete.
 *
 * @param string $queue
 *   The name of the queue to delete.
 */
function drush_advancedqueue_delete($queue = NULL) {
  // Load information about the registred queues, and sort them by weight.
  if (!$queues_info = advancedqueue_get_queues_info()) {
    return drush_set_error(dt('No queues exist.'));
  }

  $all_option = drush_get_option('all');
  if (!$all_option && empty($queue)) {
    return drush_set_error(dt('You have to specify either a set of queues or the --all parameter.'));
  }

  if ($all_option) {
    $queues = $queues_info;
  }
  else {
    // Validate queues.
    $queues = drupal_map_assoc(explode(',', $queue));
    if ($invalid_queues = array_diff_key($queues, $queues_info)) {
      return drush_set_error(dt('The following queues are invalid: !queues. Aborting.', array('!queues' => implode(', ', $invalid_queues))));
    }
    $queues = array_intersect_key($queues_info, $queues);
  }

  foreach ($queues as $queue_name => $queue_info) {
    $queue = DrupalQueue::get($queue_name);
    $queue->deleteQueue();
  }
}

/**
 * Command callback for drush advancedqueue-delete-item.
 *
 * @param $queue_name
 *   The name of the queue to delete an item from.
 * @param int $item_id
 *   The ID of the queue item to delete.
 */
function drush_advancedqueue_delete_item($queue_name, $item_id) {
  if (!in_array($queue_name, array_keys(advancedqueue_get_queues_info()))) {
    return drush_set_error(dt('The queue @queue_name does not exist.', array('@queue_name' => $queue_name)));
  }
  $queue = DrupalQueue::get($queue_name);

  $data = array(
    ':name' => $queue_name,
    ':item_id' => $item_id,
  );
  if (!$item = db_query_range('SELECT item_id, uid, title, data FROM {advancedqueue} q WHERE name = :name AND item_id = :item_id', 0, 1, $data)->fetchObject()) {
    return drush_set_error(dt('The item @item_id does not exist.', array('@item_id' => $item_id)));
  }

  if ($queue && $item) {
    $queue->deleteItem($item);

    drush_log(dt('Item @item_id deleted from queue @queue_name.', array(
      '@item_id' => $item_id,
      '@queue_name' => $queue_name,
    )), 'ok');
  }
}

/**
 * Command callback for drush advancedqueue-list-items.
 */
function drush_advancedqueue_list_items($queue = NULL) {
  // Load information about the registred queues, and sort them by weight.
  if (!$queues_info = advancedqueue_get_queues_info()) {
    return drush_set_error(dt('No queues exist.'));
  }

  $all_option = drush_get_option('all');
  if (!$all_option && empty($queue)) {
    return drush_set_error(dt('You have to specify either a set of queues or the --all parameter.'));
  }

  if ($all_option) {
    $queues = $queues_info;
  }
  else {
    // Validate queues.
    $queues = drupal_map_assoc(explode(',', $queue));
    if ($invalid_queues = array_diff_key($queues, $queues_info)) {
      return drush_set_error(dt('The following queues are invalid: !queues. Aborting.', array('!queues' => implode(', ', $invalid_queues))));
    }
    $queues = array_intersect_key($queues_info, $queues);
  }

  $rows = array(array('Queue', 'Item ID', 'Title'));

  foreach ($queues as $queue_name => $queue_info) {
    $result = db_query('SELECT name, item_id, title FROM {advancedqueue} q WHERE status <= 0 AND name = :name ORDER BY created ASC', array(':name' => $queue_name))->fetchAll();
    foreach ($result as $row) {
      $rows[] = $row;
    }
  }

  drush_print_table($rows, TRUE);

  // Return the result for backend invoke.
  return $rows;
}
