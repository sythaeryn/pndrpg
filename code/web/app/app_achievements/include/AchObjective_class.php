<?php
	class AchObjective {
		private $id;
		private $perk;
		private $condition;
		private $value;
		private $name;
		private $display;
		private $done;
		private $progress;

		function AchObjective(&$data,$lang,$user) {
			global $db;

			$this->id = $data['ao_id'];
			$this->perk = $data['ao_perk'];
			$this->condition = $data['ao_condition'];
			$this->value = $data['ao_value'];
			$this->name = $data['aol_name'];
			$this->display = $data['ao_display'];
			$this->done = $data['apo_date'];

			$this->progress = $this->value;

			if(!$this->isDone()) {
				$res = $db->sqlQuery("SELECT count(*) as anz FROM ach_player_atom,ach_atom WHERE apa_atom=atom_id AND atom_objective='".$this->id."' AND apa_player='".$user."'");
				$this->progress = $res[0]['anz'];
			}
		}

		function getID() {
			return $this->id;
		}

		function getPerk() {
			return $this->perk;
		}

		function getCondition() {
			return $this->condition;
		}

		function getValue() {
			return $this->value;
		}

		function getProgress() {
			return $this->progress;
		}

		function getName() {
			return $this->name;
		}

		function getDisplay() {
			return $this->display;
		}

		function isDone() {
			return ($this->done > 0);
		}

		function getDone() {
			return $this->done;
		}
	}
?>