<?php

/**
 * Menu callback: queue listing page.
 *
 * @see advancedqueue_ui_menu()
 */
function advancedqueue_ui_queue_list() {
  // Get all queues grouped by their 'group' property.
  $queues_by_group = advancedqueue_ui_queues_by_group();

  $advancedqueue_statuses = _advancedqueue_status_options();

  $header = array_merge(array(t('Name'), t('Machine name')), $advancedqueue_statuses, array('Operations'));

  $rows = array();
  foreach ($queues_by_group as $group_name => $queues) {
    // If we have more than one queue group defined, display group header.
    if (count($queues_by_group) > 1) {
      $rows[$group_name] = array(array(
        'data' => t($group_name),
        'colspan' => count($advancedqueue_statuses) + 3,
        'class' => 'group-name',
      ));
    }

    $group_item_count = $group_unprocessed_item_count = 0;

    foreach ($queues as $queue_name => $queue_info) {
      $result = db_query('SELECT status, COUNT(item_id) as count FROM {advancedqueue} WHERE name = :name GROUP BY status', array(':name' => $queue_name))->fetchAllKeyed();

      // Build queue name and machine name columns.
      $queue_label = !empty($queue_info['label']) ? $queue_info['label'] : $queue_name;
      $queue_label = l(t($queue_label), 'admin/structure/queues/' . $queue_name);
      if (!empty($queue_info['description'])) {
        $queue_label .= '<div class="description">' . $queue_info['description'] . '</div>';
      }
      $row = array(
        array(
          'data' => $queue_label,
          'class' => 'queue-name',
        ),
        array(
          'data' => $queue_name,
          'class' => 'queue-machine-name',
        ),
      );
      // If we have more than one group defined, indent queue names.
      if (count($queues_by_group) > 1) {
        $row[0]['class'] .= ' queue-in-group';
      }

      // Build items count columns.
      $queue_item_count = 0;
      foreach (array_keys($advancedqueue_statuses) as $status_code) {
        $row[] = array(
          'data' => isset($result[$status_code]) ? l($result[$status_code], 'admin/structure/queues/' . $queue_name, array('query' => array('status[]' => $status_code))) : '-',
          'class' => 'queue-item-count-' . strtolower($advancedqueue_statuses[$status_code]),
        );
        $queue_item_count += !empty($result[$status_code]) ? $result[$status_code] : 0;
        $group_item_count += !empty($result[$status_code]) ? $result[$status_code] : 0;
      }
      $group_unprocessed_item_count += !empty($result[ADVANCEDQUEUE_STATUS_QUEUED]) ? $result[ADVANCEDQUEUE_STATUS_QUEUED] : 0;

      // Build queue operations column.
      $operations = array();
      if (user_access('advancedqueue_ui manage queues')) {
        // Do not show 'Process' link for queues from 'Undefined' group, as we
        // don't know what their worker function is, so we can't process them.
        if (!empty($result[ADVANCEDQUEUE_STATUS_QUEUED]) && $group_name != ADVANCEDQUEUE_UI_GROUP_UNDEFINED) {
          $operations[] = l(t('process'), 'admin/structure/queues/' . $queue_name . '/process', array('attributes' => array('title' => t('Process all unprocessed items in this queue'))));
        }
        if (!empty($queue_item_count)) {
          $operations[] = l(t('delete'), 'admin/structure/queues/' . $queue_name . '/delete', array('attributes' => array('title' => t('Delete all items from this queue'))));
        }
      }
      $row[] = array(
        'data' => implode(' - ', $operations),
        'class' => 'queue-operations',
      );

      $rows[] = $row;
    }

    // Build group operations column.
    if (count($queues_by_group) > 1 && !empty($group_item_count)) {
      $rows[$group_name][0]['colspan'] = count($advancedqueue_statuses) + 2;

      $operations = array();
      if (user_access('advancedqueue_ui manage queues')) {
        // Do not show 'Process' link for 'Undefined' group, as we don't know
        // what the worker function for its queues is, so we can't process them.
        if (!empty($group_unprocessed_item_count) && $group_name != ADVANCEDQUEUE_UI_GROUP_UNDEFINED) {
          $operations[] = l(t('process'), 'admin/structure/queues/group:' . $group_name . '/process', array('attributes' => array('title' => t('Process all unprocessed items in this group'))));
        }
        $operations[] = l(t('delete'), 'admin/structure/queues/group:' . $group_name . '/delete', array('attributes' => array('title' => t('Delete all items from this group'))));
      }
      $rows[$group_name][] = array(
        'data' => implode(' - ', $operations),
        'class' => 'group-operations',
      );
    }
  }

  drupal_add_css(drupal_get_path('module', 'advancedqueue_ui') . '/advancedqueue_ui.css');

  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'advancedqueue_ui_queues'),
  ));
}

/**
 * Menu callback: queue item listing page.
 *
 * We embed the view this way instead of just adding a page display to it
 * to be able to use different page callback (advancedqueue_ui_queue_list())
 * for the "base" path. (/admin/structure/queues is the page listing all queues,
 * and /admin/structure/queues/% is the views page listing all queue items).
 *
 * @see advancedqueue_ui_menu()
 * @see advancedqueue_ui_queue_items_title()
 */
function advancedqueue_ui_queue_items($queue_name) {
  // Load the specified View.
  $view = views_get_view('advancedqueue_ui');
  $view->set_display('default');

  // Set the specific arguments passed in.
  $view->set_arguments(array($queue_name));

  // Prepare and execute the View query.
  $view->pre_execute();
  $view->execute();

  drupal_add_css(drupal_get_path('module', 'advancedqueue_ui') . '/advancedqueue_ui.css');

  // Return the rendered View.
  return $view->render();
}

/**
 * Menu title callback: provides title for queue item listing page.
 *
 * @see advancedqueue_ui_menu()
 * @see advancedqueue_ui_queue_items()
 */
function advancedqueue_ui_queue_items_title($queue_name) {
  $queues = advancedqueue_get_queues_info();
  return !empty($queues[$queue_name]['label']) ? t('@queue_label (@queue_name)', array(
    '@queue_label' => $queues[$queue_name]['label'],
    '@queue_name' => $queue_name,
  )) : $queue_name;
}

/**
 * Menu callback: queue devel page.
 *
 * @see advancedqueue_ui_menu()
 */
function advancedqueue_ui_queue_devel($queue_name) {
  $queues_info = advancedqueue_get_queues_info();
  $queue_info = $queues_info[$queue_name];

  module_load_include('pages.inc', 'devel');
  return devel_load_object('advancedqueue', $queue_info);
}


/**
 * Menu callback: queue item view page.
 *
 * @see advancedqueue_ui_menu()
 * @see advancedqueue_ui_item_view_title()
 */
function advancedqueue_ui_item_view($item_id) {
  $item = advancedqueue_ui_item_load($item_id);
  return entity_view('advancedqueue_item', array($item_id => $item), 'administrator', NULL, TRUE);
}

/**
 * Menu title callback: provides title for queue item view page.
 *
 * @see advancedqueue_ui_menu()
 * @see advancedqueue_ui_item_view()
 */
function advancedqueue_ui_item_view_title($item_id) {
  if ($item = advancedqueue_ui_item_load($item_id)) {
    return $item->title;
  }
}


/**
 * Menu callback: queue item devel page.
 *
 * @see advancedqueue_ui_menu()
 */
function advancedqueue_ui_item_devel($item_id) {
  $item = advancedqueue_ui_item_load($item_id);

  module_load_include('pages.inc', 'devel');
  return devel_load_object('advancedqueue_item', $item);
}

/**
 * Form constructor for the queue process confirmation form.
 *
 * @see advancedqueue_ui_menu()
 * @see advancedqueue_ui_queue_process_confirm_submit()
 */
function advancedqueue_ui_queue_process_confirm($form, &$form_state, $name) {
  if (strpos($name, 'group:') === 0) {
    $name = str_replace('group:', '', $name);
    $queues_by_group = advancedqueue_ui_queues_by_group();
    if (empty($queues_by_group[$name])) {
      drupal_set_message(t('No queues found for group %group_name.', array('%group_name' => $name)), 'error');
      drupal_goto('admin/structure/queues');
    }
    $type = 'group';
    $queue_names = array_keys($queues_by_group[$name]);
  }
  else {
    $type = 'queue';
    $queue_names = array($name);
  }

  $form['type'] = array('#type' => 'value', '#value' => $type);
  $form['name'] = array('#type' => 'value', '#value' => $name);
  $form['queue_names'] = array('#type' => 'value', '#value' => $queue_names);

  return confirm_form($form,
    t('Process @type %name ?', array('@type' => $type, '%name' => $name)),
    'admin/structure/queues',
    t('Are you sure you want to process all unprocessed items in @type %name ?', array('@type' => $type, '%name' => $name)),
    t('Confirm'),
    t('Cancel')
  );
}

/**
 * Form submission callback: executes queue process.
 *
 * @see advancedqueue_ui_queue_process_confirm()
 */
function advancedqueue_ui_queue_process_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    module_load_include('batch.inc', 'advancedqueue_ui', 'includes/advancedqueue_ui');
    advancedqueue_ui_batch_process($form_state['values']['queue_names'], $form_state['values']['type'], $form_state['values']['name']);
  }

  $form_state['redirect'] = 'admin/structure/queues';
}

/**
 * Form constructor for the queue deletion confirmation form.
 *
 * @see advancedqueue_ui_menu()
 * @see advancedqueue_ui_queue_delete_confirm_submit()
 */
function advancedqueue_ui_queue_delete_confirm($form, &$form_state, $name) {
  if (strpos($name, 'group:') === 0) {
    $name = str_replace('group:', '', $name);
    $queues_by_group = advancedqueue_ui_queues_by_group();
    if (empty($queues_by_group[$name])) {
      drupal_set_message(t('No queues found for group %group_name.', array('%group_name' => $name)), 'error');
      drupal_goto('admin/structure/queues');
    }
    $type = 'group';
    $queue_names = array_keys($queues_by_group[$name]);
  }
  else {
    $type = 'queue';
    $queue_names = array($name);
  }

  $form['type'] = array('#type' => 'value', '#value' => $type);
  $form['name'] = array('#type' => 'value', '#value' => $name);
  $form['queue_names'] = array('#type' => 'value', '#value' => $queue_names);

  return confirm_form($form,
    t('Delete @type %name ?', array('@type' => $type, '%name' => $name)),
    'admin/structure/queues',
    t('Are you sure you want to delete all items (including processed) from @type %name ?', array('@type' => $type, '%name' => $name)),
    t('Confirm'),
    t('Cancel')
  );
}

/**
 * Form submission callback: executes queue deletion.
 *
 * @see advancedqueue_ui_queue_delete_confirm()
 */
function advancedqueue_ui_queue_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    module_load_include('batch.inc', 'advancedqueue_ui', 'includes/advancedqueue_ui');
    advancedqueue_ui_batch_delete($form_state['values']['queue_names'], $form_state['values']['type'], $form_state['values']['name']);

    drupal_set_message(t('Deleted all items from @type %name.', array(
      '@type' => $form_state['values']['type'],
      '%name' => $form_state['values']['name'],
    )));
  }

  $form_state['redirect'] = 'admin/structure/queues';
}
