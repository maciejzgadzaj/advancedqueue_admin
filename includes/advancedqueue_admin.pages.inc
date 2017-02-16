<?php

/**
 * Menu callback: queue listing page.
 *
 * @see advancedqueue_admin_menu()
 */
function advancedqueue_admin_queue_list() {
  $queues = advancedqueue_get_queues_info();
  ksort($queues);

  $advancedqueue_statuses = _advancedqueue_status_options();

  $header = array_merge(array(t('Queue')), $advancedqueue_statuses, array(''));

  $rows = array();

  foreach (array_keys($queues) as $queue_name) {
    $result = db_query('SELECT status, COUNT(item_id) as count FROM {advancedqueue} WHERE name = :name GROUP BY status', array(':name' => $queue_name))->fetchAllKeyed();

    $row = array(
      l($queue_name, 'admin/structure/queues/' . $queue_name),
    );

    foreach (array_keys($advancedqueue_statuses) as $status_code) {
      $row[] = isset($result[$status_code]) ? l($result[$status_code], 'admin/structure/queues/' . $queue_name, array('query' => array('status' => $status_code))) : '-';
    }

    $operations = array();
    $operations[] = l(t('delete'), 'admin/structure/queues/' . $queue_name . '/delete', array('attributes' => array('title' => t('Delete all items from the queue'))));
    $row[] = implode(' - ', $operations);

    $rows[] = $row;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Menu callback: queue item listing page.
 *
 * We embed the view this way instead of just adding a page display to it
 * to be able to use different page callback (advancedqueue_admin_queue_list())
 * for the "base" path. (/admin/structure/queues is the page listing all queues,
 * and /admin/structure/queues/% is the views page listing all queue items).
 *
 * @see advancedqueue_admin_menu()
 * @see advancedqueue_admin_queue_items_title()
 */
function advancedqueue_admin_queue_items($queue_name) {
  // Load the specified View.
  $view = views_get_view('advancedqueue_admin');
  $view->set_display('default');

  // Set the specific arguments passed in.
  $view->set_arguments(array($queue_name));

  // Prepare and execute the View query.
  $view->pre_execute();
  $view->execute();

  // Return the rendered View.
  return $view->render();
}

/**
 * Menu title callback: provides title for queue item listing page.
 *
 * @see advancedqueue_admin_menu()
 * @see advancedqueue_admin_queue_items()
 */
function advancedqueue_admin_queue_items_title($queue_name) {
  $queues = advancedqueue_get_queues_info();
  return !empty($queues[$queue_name]['label']) ? t('@queue_label (@queue_name)', array(
    '@queue_label' => $queues[$queue_name]['label'],
    '@queue_name' => $queue_name,
  )) : $queue_name;
}

/**
 * Menu callback: queue devel page.
 *
 * @see advancedqueue_admin_menu()
 */
function advancedqueue_admin_queue_devel($queue_name) {
  $queues_info = advancedqueue_get_queues_info();
  $queue_info = $queues_info[$queue_name];

  module_load_include('pages.inc', 'devel');
  return devel_load_object('advancedqueue', $queue_info);
}


/**
 * Menu callback: queue item view page.
 *
 * @see advancedqueue_admin_menu()
 * @see advancedqueue_admin_item_view_title()
 */
function advancedqueue_admin_item_view($item_id) {
  $item = advancedqueue_admin_item_load($item_id);
  return entity_view('advancedqueue_item', array($item_id => $item), 'administrator', NULL, TRUE);
}

/**
 * Menu title callback: provides title for queue item view page.
 *
 * @see advancedqueue_admin_menu()
 * @see advancedqueue_admin_item_view()
 */
function advancedqueue_admin_item_view_title($item_id) {
  if ($item = advancedqueue_admin_item_load($item_id)) {
    return $item->title;
  }
}


/**
 * Menu callback: queue item devel page.
 *
 * @see advancedqueue_admin_menu()
 */
function advancedqueue_admin_item_devel($item_id) {
  $item = advancedqueue_admin_item_load($item_id);

  module_load_include('pages.inc', 'devel');
  return devel_load_object('advancedqueue_item', $item);
}

/**
 * Form constructor for the queue deletion confirmation form.
 *
 * @see advancedqueue_admin_menu()
 * @see advancedqueue_admin_queue_delete_confirm_submit()
 */
function advancedqueue_admin_queue_delete_confirm($form, &$form_state, $queue_name) {
  $form['queue_name'] = array('#type' => 'value', '#value' => $queue_name);
  return confirm_form($form,
    t('Delete %queue_name ?', array('%queue_name' => $queue_name)),
    'admin/structure/queues',
    t('Are you sure you want to delete all items (including processed) from queue %queue_name ?', array('%queue_name' => $queue_name)),
    t('Confirm'),
    t('Cancel')
  );
}

/**
 * Form submission callback: executes queue deletion.
 *
 * @see advancedqueue_admin_queue_delete_confirm()
 */
function advancedqueue_admin_queue_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $queue_name = $form_state['values']['queue_name'];
    $queue = DrupalQueue::get($queue_name);
    $queue->deleteQueue();

    drupal_set_message(t('Deleted all items from queue %queue_name.', array('%queue_name' => $queue_name)));
  }

  $form_state['redirect'] = 'admin/structure/queues';
}