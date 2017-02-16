<?php

function advancedqueue_admin_pages_queue_list() {
  $queues = advancedqueue_get_queues_info();
  ksort($queues);

  $advancedqueue_statuses = array(
    -1 => 'queued',
    0 => 'processing',
    1 => 'success',
    2 => 'failure',
    3 => 'failure retry',
  );

  $header = array(
    t('Queue'),
    t('Queued'),
    t('Processing'),
    t('Success'),
    t('Failure'),
    t('Failure retry'),
  );

  $rows = array();

  foreach (array_keys($queues) as $queue_name) {
    $result = db_query('SELECT status, COUNT(item_id) as count FROM {advancedqueue} WHERE name = :name GROUP BY status', array(':name' => $queue_name))->fetchAllKeyed();

    $row = array(
      l($queue_name, 'admin/structure/queues/' . $queue_name),
    );

    foreach (array_keys($advancedqueue_statuses) as $status_code) {
      $row[] = isset($result[$status_code]) ? l($result[$status_code], 'admin/structure/queues/' . $queue_name, array('query' => array('status' => $status_code))) : '-';
    }

    $rows[] = $row;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}
