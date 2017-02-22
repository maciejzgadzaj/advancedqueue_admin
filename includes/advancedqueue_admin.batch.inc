<?php

/**
 * Creates a batch to process all unprocessed items from specified queues.
 *
 * @param array $queue_names
 *   An array of queue names to process the unprocessed items from.
 *
 * @see advancedqueue_admin_queue_process_confirm_submit()
 */
function advancedqueue_admin_batch_process($queue_names, $type, $name) {
  $item_ids = db_query('SELECT item_id FROM {advancedqueue} q WHERE status = -1 AND expire = 0 AND name IN (:names) AND created <= :created ORDER BY created ASC', array(
    ':created' => time(),
    ':names' => $queue_names,
  ))->fetchCol();

  if (!empty($item_ids)) {
    $batch = array(
      'title' => t('Processing items from @type %name.', array('@type' => $type, '%name' => $name)),
      'operations' => array(),
      'progress_message' => t('Processed @current out of @total'),
      'finished' => 'advancedqueue_admin_batch_process_finished',
      'file' => drupal_get_path('module', 'advancedqueue_admin') . '/includes/advancedqueue_admin.batch.inc',
    );

    foreach ($item_ids as $item_id) {
      $batch['operations'][] = array('advancedqueue_admin_batch_process_operation', array($item_id));
    }

    batch_set($batch);
  }
}

/**
 * Implements callback_batch_operation().
 *
 * Processed a single unprocessed queue item.
 *
 * @see advancedqueue_admin_batch_process()
 */
function advancedqueue_admin_batch_process_operation($item_id) {
  $item = advancedqueue_admin_item_load($item_id);
  module_load_include('actions.inc', 'advancedqueue_admin');
  advancedqueue_admin_process_item_action($item);
}

/**
 * Implements callback_batch_finished().
 *
 * Final batch callback for processing unprocessed items from a queue.
 *
 * @see advancedqueue_admin_batch_process()
 */
function advancedqueue_admin_batch_process_finished($success, $results, $operations, $elapsed) {
  if (!$success) {
    $error_operation = reset($operations);
    drupal_set_message(t('An error occurred while processing @operation with arguments : @args', array('@operation' => $error_operation[0], '@args' => print_r($error_operation[0], TRUE))));
  }
}

/**
 * Creates a batch to delete all items from a queue/group.
 *
 * @param array $queue_names
 *   An array of queue names to delete items from.
 *
 * @see advancedqueue_admin_queue_delete_confirm_submit()
 */
function advancedqueue_admin_batch_delete($queue_names, $type, $name) {
  if (!empty($queue_names)) {
    $batch = array(
      'title' => t('Deleting items from @type %name.', array('@type' => $type, '%name' => $name)),
      'operations' => array(),
      'progress_message' => t('Processed @current out of @total'),
      'finished' => 'advancedqueue_admin_batch_delete_finished',
      'file' => drupal_get_path('module', 'advancedqueue_admin') . '/includes/advancedqueue_admin.batch.inc',
    );

    foreach ($queue_names as $queue_name) {
      $batch['operations'][] = array('advancedqueue_admin_batch_delete_operation', array($queue_name));
    }

    batch_set($batch);
  }
}

/**
 * Implements callback_batch_operation().
 *
 * Deletes all items from a single queue.
 *
 * @see advancedqueue_admin_batch_delete()
 */
function advancedqueue_admin_batch_delete_operation($queue_name) {
  $queue = DrupalQueue::get($queue_name);
  $queue->deleteQueue();
}

/**
 * Implements callback_batch_finished().
 *
 * Final batch callback for deleting all items from a queue/group.
 *
 * @see advancedqueue_admin_batch_delete()
 */
function advancedqueue_admin_batch_delete_finished($success, $results, $operations, $elapsed) {
  if (!$success) {
    $error_operation = reset($operations);
    drupal_set_message(t('An error occurred while processing @operation with arguments : @args', array('@operation' => $error_operation[0], '@args' => print_r($error_operation[0], TRUE))));
  }
}
