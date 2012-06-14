<?php
	class AchCategory extends AchList implements Tieable {
		private $id = false;
		private $ties_cult;
		private $ties_civ;
		private $ties_cult_dev;
		private $ties_civ_dev;
		private $cult;
		private $civ;

		function AchCategory($id,$cult = null,$civ = null) {
			global $DBc,$_USER;

			if($cult == null) {
				$cult = $_USER->getCult();
			}

			if($civ == null) {
				$civ = $_USER->getCiv();
			}

			$this->cult = $cult;
			$this->civ = $civ;

			$this->id = mysql_real_escape_string($id);

			$res = $DBc->sqlQuery("SELECT * FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$_USER->getLang()."' AND aal_achievement=aa_id) WHERE aa_category='".$this->id."' AND (aa_parent IS NULL OR NOT EXISTS (SELECT * FROM ach_perk WHERE ap_achievement=aa_id AND NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_player='".$_USER->getID()."' AND app_perk=ap_id))) AND (aa_tie_race IS NULL OR aa_tie_race='".$_USER->getRace()."') AND (aa_tie_cult IS NULL OR aa_tie_cult='".mysql_real_escape_string($cult)."') AND (aa_tie_civ IS NULL OR aa_tie_civ='".mysql_real_escape_string($civ)."') ORDER by aal_name ASC");

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				#echo "Y";
				$tmp = new AchAchievement($res[$i]);
				#echo var_export($tmp,true);
				if($tmp->hasOpen()) {
					$this->child_open[] = sizeof($this->nodes);
				}
				if($tmp->hasDone()) {
					$this->child_done[] = sizeof($this->nodes);
				}

				$this->nodes[] = $tmp;
			}

			$res = $DBc->sqlQuery("SELECT count(*) as anz FROM ach_achievement WHERE aa_tie_cult IS NOT NULL AND aa_category='".$this->id."' AND aa_dev='0'");
			$this->ties_cult = $res[0]['anz'];

			$res = $DBc->sqlQuery("SELECT count(*) as anz FROM ach_achievement WHERE aa_tie_civ IS NOT NULL AND aa_category='".$this->id."' AND aa_dev='0'");
			$this->ties_civ = $res[0]['anz'];

			$res = $DBc->sqlQuery("SELECT count(*) as anz FROM ach_achievement WHERE aa_tie_cult IS NOT NULL AND aa_category='".$this->id."'");
			$this->ties_cult_dev = $res[0]['anz'];

			$res = $DBc->sqlQuery("SELECT count(*) as anz FROM ach_achievement WHERE aa_tie_civ IS NOT NULL AND aa_category='".$this->id."'");
			$this->ties_civ_dev = $res[0]['anz'];
		}

		function getID() {
			return $this->id;
		}

		function isTiedCult() {
			return ($this->ties_cult > 0);
		}

		function isTiedCiv() {
			return ($this->ties_civ > 0);
		}

		function isTiedCultDev() {
			return ($this->ties_cult_dev > 0);
		}

		function isTiedCivDev() {
			return ($this->ties_civ_dev > 0);
		}

		function getCurrentCiv() {
			return $this->civ;
		}

		function getCurrentCult() {
			return $this->cult;
		}
	}
?>