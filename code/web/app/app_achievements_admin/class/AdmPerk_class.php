<?php
	class AdmPerk extends AchPerk implements ADM {
		function insertNode($n) {
			$n->setParent($this);
			$n->insert();
			$this->addChild($n);
		}

		function removeNode($id) {
			$res = $this->getChildDataByID($id);
			if($res != null) {
				$res->delete_me();
				$this->removeChild($id);
			}
		}

		function updateNode($id) { // PROBABLY USELESS!
			$res = $this->getChildDataByID($id);
			if($res != null) {
				$res->update();
			}
		}

		function getPathID($path = "") {
			if($path != "") {
				$path = ";".$path;
			}
			$path = $this->getID().$path;
			if($this->parent != null) {
				return $this->parent->getPathID($path);
			}

			return $path;
		}

		function getElementByPath($pid) {
			$tmp = explode(";",$pid);
			if($tmp[0] == $this->getID()) {
				if(sizeof($tmp) > 1) {
					$c = $this->getChildDataByID($tmp[1]);
					if($c != null) {
						unset($tmp[0]);
						return $c->getElementByPath(implode(";",$tmp));
					}
					return null;
				}
				else {
					return $this;
				}
			}
			return null;
		}

		protected $condition;
		protected $condition_value;
		protected $porder;

		function AdmPerk($data,$parent) {
			parent::__construct($data,$parent);

			$this->condition = $data["ap_condition"];
			$this->condition_value = $data["ap_condition_value"];
			$this->porder = $data["ap_porder"];
		}

		protected function makeChild($d) {
			return new AdmObjective($d,$this);
		}

		function delete_me() {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_perk WHERE ap_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_player_perk WHERE app_perk='".$this->getID()."'");

			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->delete_me();
				$this->removeChild($curr->getID());
			}
		}

		function update() {
			global $DBc;

			$DBc->sqlQuery("UPDATE ach_perk SET ap_parent=".mkn($this->getParentID()).",ap_value='".$DBc->sqlEscape($this->getValue())."',ap_condition='".$DBc->sqlEscape($this->getCondition())."',ap_condition_value=".mkn($this->getConditionValue()).",ap_dev='".$this->getDev()."',ap_porder='".$this->porder."' WHERE ap_id='".$this->getID()."'");

			$DBc->sqlQuery("INSERT INTO ach_perk_lang (apl_perk,apl_lang,apl_name,apl_template) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."',".mkn($this->getTemplate()).") ON DUPLICATE KEY UPDATE apl_name='".$DBc->sqlEscape($this->getName())."',apl_template=".mkn($this->getTemplate())."");
		}

		function insert() {
			global $DBc;

			$this->dev = 1;

			$DBc->sqlQuery("INSERT INTO ach_perk (ap_achievement,ap_parent,ap_value,ap_condition,ap_condition_value,ap_dev,ap_porder) VALUES ('".$this->getAchievement()."',".mkn($this->getParentID()).",'".$DBc->sqlEscape($this->getValue())."','".$DBc->sqlEscape($this->getCondition())."',".mkn($this->getConditionValue()).",'1','".$this->porder."')");
			$id = $DBc->insertID();
			$this->setID($id);

			$DBc->sqlQuery("INSERT INTO ach_perk_lang (apl_perk,apl_lang,apl_name,apl_template) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."',".mkn($this->getTemplate()).")");
		}

		function setAchievement($a) {
			$this->achievement = $a;
		}

		function setName($name) {
			$this->name = $name;
		}

		function setTemplate($t) {
			$this->template = $t;
		}

		function setValue($v) {
			$this->value = $v;
		}

		function getCondition() {
			return $this->condition;
		}

		function getConditionValue() {
			return $this->condition_value;
		}

		function setCondition($c) {
			$this->condition = $c;
		}

		function setConditionValue($v) {
			$this->condition_value = $v;
		}

		function getPorder() {
			return $this->porder;
		}

		function setPorder($p) {
			$this->porder = $p;
		}

		private function reOrder() {
			//check if order is OK!

			if($this->parent_id == null) {

			}
			else {

			}
		}

		function setParentID($p) { #reordering must happen A) after insert B) when updating
			if($p == null || $p == "null") {
				//remove from ach list; insert as first!
				$this->parent_id = null;

				#$this->parent->removeChild($this->id);
				#$iter = $this->parent->getIterator();
				#$this->parent->addOpen($this,$iter->getNext());
			}
			else {
				//remove from ach list; insert after parent
				#echo "--".$p."<br>";
				$this->parent_id = $p;

				#$this->parent->removeChild($this->id);
				#$item = $this->parent->getChildByID($this->parent_id);
				#$tmp = $item->getChild();
				#if($tmp != null) {
				#	$this->parent->addOpen($this,$tmp->getID());
				#}
				#else {
				#	$this->parent->addOpen($this,null);
				#}
			}
		}
	}
?>