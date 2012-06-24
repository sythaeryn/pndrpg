<?php
	abstract class RenderNodeIterator extends Parentum {
		protected $nodes = array();

		function getSize() {
			return sizeof($this->nodes);
		}

		function getChild($i) {
			return $this->nodes[$i];
		}

		function isEmpty() {
			return (sizeof($this->nodes) == 0);
		}

		function getChildren() {
			return $this->nodes;
		}
	}
?>