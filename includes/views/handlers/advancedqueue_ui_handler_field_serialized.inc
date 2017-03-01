<?php

/**
 * @file
 * Definition of views_handler_field_serialized.
 */

/**
 * Field handler to show data of serialized fields.
 *
 * @ingroup views_field_handlers
 */
class advancedqueue_ui_handler_field_serialized extends views_handler_field_serialized {

  function render($values) {
    $value = $values->{$this->field_alias};

    if ($this->options['format'] == 'unserialized') {
      return check_plain(print_r(unserialize($value), TRUE));
    }
    elseif ($this->options['format'] == 'key' && !empty($this->options['key'])) {
      $value = (array) unserialize($value);
      return isset($value[$this->options['key']]) ? check_plain($value[$this->options['key']]) : '';
    }

    return $value;
  }
}
