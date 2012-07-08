<?php
	abstract class AchList extends Parentum {
		/*---------------------------
			This class organizes nodes to distinguish between "open" and "done" nodes.

			child_open and child_done refer to the index set in Parentum::nodes[]
		---------------------------*/

		protected $child_done;
		protected $child_open;

		function AchList() {
			parent::__construct();

			$this->child_done = new DLL();
			$this->child_open = new DLL();
		}

		final function getDone() {
			return $this->child_done->getIterator();
		}

		final function getOpen() {
			return $this->child_open->getIterator();
		}

		final function hasOpen() {
			#echo "hasOpen: ".$this->child_open->getSize()."<br>";
			return ($this->child_open->getSize() != 0);
		}

		final function hasDone() {
			#echo "hasDone: ".$this->child_done->getSize()."<br>";
			return ($this->child_done->getSize() != 0);
		}

		final function addOpen($data) {
			$this->child_open->addNode($data);
			$this->addChild($data); #Parentum::addChild()
		}

		final function addDone($data) {
			$this->child_done->addNode($data);
			$this->addChild($data); #Parentum::addChild()
		}

		final function setChildDone($id) {
			$this->addChildDone($id);
			$this->removeChildOpen($id);
		}

		final function setChildOpen($id) {
			$this->addChildOpen($id);
			$this->removeChildDone($id);
		}

		final function addChildDone($id) {
			$data = $this->getChildDataByID($id);
			$this->child_done->addNode($data);
		}

		final function addChildOpen($id) {
			$data = $this->getChildDataByID($id);
			$this->child_open->addNode($data);
		}

		final function removeChildDone($id) {
			$this->child_done->removeNode($id);
		}

		final function removeChildOpen($id) {
			$this->child_open->removeNode($id);
		}
		
		#@OVERRIDE Parentum::removeChild()
		function removeChild($id) {
			parent::removeChild($id);
			
			$this->child_open->removeNode($id);
			$this->child_done->removeNode($id);
		}
	}
?>